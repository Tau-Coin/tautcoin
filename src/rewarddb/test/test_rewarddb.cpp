// Copyright (c) 2017-2018 The Taucoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "test_rewarddb.h"

//#include "chainparams.h"
//#include "consensus/consensus.h"
//#include "consensus/validation.h"
#include "key.h"
#include "main.h"
//#include "miner.h"
//#include "pubkey.h"
//#include "random.h"
//#include "txdb.h"
//#include "txmempool.h"
//#include "ui_interface.h"
//#include "rpc/server.h"
//#include "rpc/register.h"

#include "test/testutil.h"

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <list>
#include <map>
#include <set>
#include <stdlib.h>
#include <time.h>
#include <algorithm>

//extern bool fPrintToConsole;
extern void noui_connect();

using namespace std;
namespace TAURewardDB
{
BasicTestingSetup::BasicTestingSetup(const std::string& chainName)
{
        ECC_Start();
        SetupEnvironment();
        SetupNetworking();
        fPrintToDebugLog = false; // don't want to write to debug.log file
        fCheckBlockIndex = true;
        SelectParams(chainName);
        noui_connect();
}

BasicTestingSetup::~BasicTestingSetup()
{
        ECC_Stop();
}

TestingSetup::TestingSetup(const std::string& chainName) : BasicTestingSetup(chainName)
{
    const CChainParams& chainparams = Params();
    // Ideally we'd move all the RPC tests to the functional testing framework
    // instead of unit tests, but for now we need these here.
    //RegisterAllCoreRPCCommands(tableRPC);
    ClearDatadirCache();
    pathTemp = GetTempPath() / strprintf("test_bitcoin_%lu_%i", (unsigned long)GetTime(), (int)(GetRand(100000)));
    boost::filesystem::create_directories(pathTemp);
    mapArgs["-datadir"] = pathTemp.string();
    pclubinfodb = new CClubInfoDB(0);
    paddrinfodb = new CAddrInfoDB(0, pclubinfodb);
    //mempool.setSanityCheck(1.0);
    pblocktree = new CBlockTreeDB(1 << 20, true);
    pcoinsdbview = new CCoinsViewDB(1 << 23, true);
    pcoinsTip = new CCoinsViewCache(pcoinsdbview);
    InitBlockIndex(chainparams);
    nScriptCheckThreads = 3;
    for (int i=0; i < nScriptCheckThreads-1; i++)
        threadGroup.create_thread(&ThreadScriptCheck);
    RegisterNodeSignals(GetNodeSignals());
}

TestingSetup::~TestingSetup()
{
        UnregisterNodeSignals(GetNodeSignals());
        threadGroup.interrupt_all();
        threadGroup.join_all();
        UnloadBlockIndex();
        delete pcoinsTip;
        delete pcoinsdbview;
        delete pblocktree;
        delete paddrinfodb;
        paddrinfodb = NULL;
        delete pclubinfodb;
        pclubinfodb = NULL;
        boost::filesystem::remove_all(pathTemp);
}

}

static string GetRandomAddress()
{
    CKey secret;
    secret.MakeNewKey(true);
    CPubKey pubkey = secret.GetPubKey();

    //std::cout<<"{PublicKey: \""<<HexStr(ToByteVector(pubkey))<<"\","<<std::endl;
    //std::cout<<"PrivateKey: \""<<CBitcoinSecret(secret).ToString()<<"\","<<std::endl;

    const CScript genesisOutputScript0 = CScript() << ParseHex(HexStr(ToByteVector(pubkey))) << OP_CHECKSIG;
    CBitcoinAddress addr;
    std::string address;
    addr.ScriptPub2Addr(genesisOutputScript0, address);
    //std::cout<<"Address: \""<<address<<"\"}"<<std::endl;

    return address;
}

BOOST_FIXTURE_TEST_SUITE(crewarddb_tests, TAURewardDB::TestingSetup)

//static const unsigned int NUM_SIMULATION_ITERATIONS = 40000;
//static const unsigned int NUM_SIMULATION_ADDRESSS = 1000000;

//BOOST_AUTO_TEST_CASE(rewarddb_performance_simulation_test)
//{
//    int num = 100000;
//    vector<string> addresses;
//    vector<string> addressesRandom;
//    int numRandomly = 0;
//    for(int i = 0; i < num; i++)
//    {
//        srand((unsigned)std::time(0));
//        bool random = ((rand()%5) == 0);
//        string addr = GetRandomAddress();
//        addresses.push_back(addr);
//        if (i != 0)
//        {
//            for(int h = 0; h < 10; h++)
//                paddrinfodb->Write(make_pair(addr, h), CTAUAddrInfo());
//        }
//        if (random && numRandomly < num/5)
//        {
//            addressesRandom.push_back(addr);
//            numRandomly++;
//        }
//    }
//    paddrinfodb->InitGenesisDB(addresses);
//    paddrinfodb->Commit(9);

//    int test1Num = num;
//    std::clock_t start, finish;
//    start = clock();
//    double totaltime = 0;
//    for(size_t t = 0; t < addressesRandom.size(); t++, test1Num*=10)
//    {
//        paddrinfodb->UpdateMpAndTotalMPByAddress(addressesRandom[t], 10, addresses[0]);
//    }
//    vector<string> members;
//    pclubinfodb->GetTotalMembersByAddress(addresses[0], members);
//    paddrinfodb->Commit(10);

//    finish = clock();
//    totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
//    cout<<"When updating "<<addressesRandom.size()<<" items, time comsume is "<<totaltime<<"s！"<<endl;
//}

BOOST_AUTO_TEST_CASE(clubInfodb_UpdateMembersByFatherAddress_test)
{
    // Init
    vector<string> addresses;
    int num = 5;
    LOCK(cs_clubinfo);
    for(int i = 0; i < num; i++)
    {
        srand((unsigned)std::time(0));
        string addr = GetRandomAddress();
        addresses.push_back(addr);
    }

    // Create inputs
    const int addSize = 2;
    const int fatherAddressSize = 2;
    const int memberinfoSize = 2;
    const int indexInSize = 4;
    const int cacheRecordSize = 4;
    const bool add[addSize] = {true, false};
    const string fatherAddress[fatherAddressSize] = {"TP6t7swv4SDcDuN5pQxdk4MGGEcVpsExHG",
                                                     "TNELcnfUUak1J1Cw1bUdF3UBXunfR71Hmb"};
    CMemberInfo memberinfo[memberinfoSize];
    memberinfo[0] = CMemberInfo("TNELcnfUUak1J1Cw1bUdF3UBXunfR71Hmb", 1, 1);
    memberinfo[1] = CMemberInfo("THghrLpjMqNztfwEkT2ayWeopDo5vmnbLU", 1, 1);
    const uint64_t indexIn[indexInSize] = {0, 1, 2, 3};

    // Create true outputs
    uint64_t indexRet[fatherAddressSize][memberinfoSize][cacheRecordSize] = {0};
    for(int j = 0; j < fatherAddressSize; j++)
    {
        for(int k = 0; k < memberinfoSize; k++)
        {
            for(int m = 0; m < cacheRecordSize; m++)
            {
                if (j == 0)
                    indexRet[j][k][m] = 1;
                else if (j == 1 && k == 1)
                    indexRet[j][k][m] = 1 + m;
                else
                    indexRet[j][k][m] = 0;
            }
        }
    }
    bool recordErased[fatherAddressSize][memberinfoSize][indexInSize][cacheRecordSize] = {false};
    string addressMovedRet[fatherAddressSize][memberinfoSize][indexInSize][cacheRecordSize];
    for(int m = 0; m < cacheRecordSize; m++)
    {
        for(int j = 0; j < fatherAddressSize; j++)
        {
            for(int k = 0; k < memberinfoSize; k++)
            {
                for(int l = 0; l < indexInSize; l++)
                {
                    if ((j == 0) || (j == 1 && m >= 1 && l >= m) || (j == 1 && l == 0))
                        addressMovedRet[j][k][l][m] = NO_MOVED_ADDRESS;
                    else if(m == 0)
                    {
                        recordErased[j][k][l][m] = true;
                        addressMovedRet[j][k][l][m] = NO_MOVED_ADDRESS;
                    }
                    else if(l < m)
                        addressMovedRet[j][k][l][m] = addresses[m-1];
                }
            }
        }
    }

    // Execute tests
    uint64_t indexOut[fatherAddressSize][memberinfoSize][indexInSize][cacheRecordSize] = {0};
    CMemberInfo memberinfoOut[addSize][fatherAddressSize][memberinfoSize][indexInSize][cacheRecordSize]
            = {CMemberInfo()};
    string addressMovedOut[fatherAddressSize][memberinfoSize][indexInSize][cacheRecordSize];
    for(int m = 0; m < cacheRecordSize; m++)
    {
        for(int i = 0; i < addSize; i++)
        {
            for(int j = 0; j < fatherAddressSize; j++)
            {
                for(int k = 0; k < memberinfoSize; k++)
                {
                    for(int l = 0; l < indexInSize; l++)
                    {
                        // Init
                        paddrinfodb->ClearReadCache();
                        pclubinfodb->ClearCache();
                        string father = "TNELcnfUUak1J1Cw1bUdF3UBXunfR71Hmb";
                        vector<string> inits;
                        inits.push_back(father);
                        paddrinfodb->InitGenesisDB(inits);
                        uint64_t index = 0;
                        for(int n = 0; n < m; n++)
                            paddrinfodb->UpdateMpAndTotalMPByAddress(addresses[n], 1, father);
                        paddrinfodb->Commit(1);
                        assert(pclubinfodb->GetCacheRecord(father).size() == size_t(m+1));

                        if (i == 0)
                        {
                            index = indexIn[l];
                            pclubinfodb->UpdateMembersByFatherAddress(fatherAddress[j],
                                                                      memberinfo[k],
                                                                      index, 2, true);

                            indexOut[j][k][l][m] = index;
                            memberinfoOut[i][j][k][l][m] =
                                    (pclubinfodb->GetCacheRecord(fatherAddress[j]))[indexOut[j][k][l][m]];
                        }
                        else
                        {
                            index = indexIn[l];
                            addressMovedOut[j][k][l][m] =
                                    pclubinfodb->UpdateMembersByFatherAddress(fatherAddress[j],
                                                                              CMemberInfo(" ", 0, 0),
                                                                              index, 2, add[i]);

                            if (j == 1 && recordErased[j][k][l][m] == false)
                            {
                                if (pclubinfodb->GetCacheRecord(fatherAddress[j]).size() > indexIn[l])
                                    memberinfoOut[i][j][k][l][m] =
                                        (pclubinfodb->GetCacheRecord(fatherAddress[j]))[indexIn[l]];
                            }
                        }

                        paddrinfodb->ClearReadCache();
                        pclubinfodb->ClearCache();
                    }
                }
            }
        }
    }

    // Verify
    for(int m = 0; m < cacheRecordSize; m++)
    {
        for(int i = 0; i < addSize; i++)
        {
            for(int j = 0; j < fatherAddressSize; j++)
            {
                for(int k = 0; k < memberinfoSize; k++)
                {
                    for(int l = 0; l < indexInSize; l++)
                    {
                        if (i == 0)
                        {
                            BOOST_CHECK_MESSAGE(indexOut[j][k][l][m] == indexRet[j][k][m], "index case: "<<j<<k<<m);
                            BOOST_CHECK_MESSAGE(memberinfoOut[i][j][k][l][m].address.compare(memberinfo[k].address) == 0,
                                                "i = 0, member case: "<<j<<k<<l<<m);
                        }
                        else
                        {
                            BOOST_CHECK_MESSAGE(addressMovedOut[j][k][l][m] == addressMovedRet[j][k][l][m], "i = 1, member case: "<<j<<k<<l<<m);
                            if (addressMovedOut[j][k][l][m].compare("NO_MOVED_ADDRESS") != 0)
                                BOOST_CHECK_MESSAGE(addressMovedOut[j][k][l][m].compare(memberinfoOut[i][j][k][l][m].address) == 0,
                                                    "address moved case: "<<j<<k<<l<<m);
                        }

                    }
                }
            }
        }
    }

}

BOOST_AUTO_TEST_CASE(addrInfodb_EntrustByAddress_test)
{
    vector<string> addresses;
    int num = 1;
    for(int i = 0; i < num; i++)
    {
        srand((unsigned)std::time(0));
        //bool random = ((rand()%5) == 0);
        string addr = GetRandomAddress();
        addresses.push_back(addr);
    }

    LOCK(cs_clubinfo);
    string testFather = "TNELcnfUUak1J1Cw1bUdF3UBXunfR71Hmb";
    paddrinfodb->UpdateMpAndTotalMPByAddress(addresses[0], 1, testFather);
    cout<<addresses[0]<<endl;
    paddrinfodb->Commit(1);

    vector<string> members;
    pclubinfodb->GetTotalMembersByAddress(testFather, members);
    for(size_t i = 0; i < members.size(); i++)
        cout<<members[i]<<endl;
}

BOOST_AUTO_TEST_SUITE_END()
