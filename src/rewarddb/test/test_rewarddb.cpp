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
#include "rewarddb/addrtrie.h"

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

BOOST_FIXTURE_TEST_SUITE(rewarddb_tests, TAURewardDB::TestingSetup)

static const unsigned int NUM_SIMULATION_ITERATIONS = 40000;
static const unsigned int NUM_SIMULATION_ADDRESSS = 1000000;

BOOST_AUTO_TEST_CASE(coins_cache_simulation_test)
{
    int num = 7211;
    int numRandomly = num / 2;
    string test;
    //input
    vector<string> addresses;
    vector<string> addressesRandom;
    for(int i = 0, j = 0; i < num; i++)
    {
        srand((unsigned)std::time(0));
        bool random = rand()%2;
        string addr = GetRandomAddress();
        addresses.push_back(addr);
        if (random && j < numRandomly)
        {
            addressesRandom.push_back(addr);
            j++;
        }
    }

    // test 1, traversal: set map list vector
    int test1Num = num;
    cout<<"traversal test"<<endl;
    for(int t = 0; t < 1; t++, test1Num*=10)
    {
        std::clock_t start, finish;
        double totaltime = 0;
//        //set
//        set<string> s;
//        for(int i = 0; i < test1Num; i++)
//            s.insert(addresses[i]);

//        start = clock();
//        for(set<string>::iterator ite = s.begin(); ite != s.end(); ite++)
//            test = *ite;
//        finish = clock();
//        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
//        cout<<"set_"<<test1Num<<" 的traversal时间为"<<totaltime<<"秒！"<<endl;

        //map
        map<string, string> m;
        for(int i = 0; i < test1Num; i++)
            m.insert(pair<string, string>(addresses[i], addresses[i]));

        start = clock();
        for(map<string, string>::iterator ite = m.begin(); ite != m.end(); ite++)
            test = ite->second;
        finish = clock();
        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
        cout<<"map_"<<test1Num<<" 的traversal时间为"<<totaltime<<"秒！"<<endl;

//        //list
//        list<string> l;
//        for(int i = 0; i < test1Num; i++)
//            l.push_back(addresses[i]);

//        start = clock();
//        for(list<string>::iterator ite = l.begin(); ite != l.end(); ite++)
//            test = *ite;
//        finish = clock();
//        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
//        cout<<"list_"<<test1Num<<" 的traversal时间为"<<totaltime<<"秒！"<<endl;

        //vector
        vector<string> v;
        v.reserve(test1Num*10);
        for(int i = 0; i < test1Num; i++)
            v.push_back(addresses[i]);

        start = clock();
        for(size_t k = 0; k < v.size(); k++)
            test = v[k];
        finish = clock();
        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
        cout<<"vector_"<<test1Num<<" 的traversal时间为"<<totaltime<<"秒！"<<endl;

        start = clock();
        for(size_t k = 0; k < v.size(); k++)
            v[k] = test;//test = v[k];
        finish = clock();
        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
        cout<<"vector_"<<test1Num<<" 的traversal2时间为"<<totaltime<<"秒！"<<endl;
    }

    // test 2, read randomly: set map list vector TAUAddrTrie
    int test2Num = num;
    cout<<"read_randomly test"<<endl;
    for(int t = 0; t < 1; t++, test2Num*=10)
    {
        //set
        set<string> s;
        for(int i = 0; i < test2Num; i++)
            s.insert(addresses[i]);

        std::clock_t start, finish;
        start = clock();
        set<string>::iterator ite;
        for(size_t i = 0; i < addressesRandom.size(); i++)
        {
            ite = s.find(addressesRandom[i]);
            test = *ite;
        }
        finish = clock();
        double totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
        cout<<"set_"<<addressesRandom.size()<<" 的read_randomly时间为"<<totaltime<<"秒！"<<endl;

        //map
        map<string, string> m;
        for(int i = 0; i < test2Num; i++)
            m.insert(pair<string, string>(addresses[i], addresses[i]));

        start = clock();
        for(size_t i = 0; i < addressesRandom.size(); i++)
            test = m[addressesRandom[i]];
        finish = clock();
        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
        cout<<"map_"<<addressesRandom.size()<<" 的read_randomly时间为"<<totaltime<<"秒！"<<endl;

        //list
        list<string> l;
        for(int i = 0; i < test2Num; i++)
            l.push_back(addresses[i]);

        start = clock();
        for(size_t i = 0; i < addressesRandom.size(); i++)
        {
            list<string>::iterator ite = find(l.begin(), l.end(), addressesRandom[i]);
            test = *ite;
        }
        finish = clock();
        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
        cout<<"list_"<<addressesRandom.size()<<" 的read_randomly时间为"<<totaltime<<"秒！"<<endl;

        //vector
        vector<string> v;
        for(int i = 0; i < test2Num; i++)
            v.push_back(addresses[i]);

        start = clock();
        for(size_t i = 0; i < addressesRandom.size(); i++)
        {
            vector<string>::iterator ite = find(v.begin(), v.end(), addressesRandom[i]);
            test = *ite;
        }
        finish = clock();
        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
        cout<<"vector_"<<addressesRandom.size()<<" 的read_randomly时间为"<<totaltime<<"秒！"<<endl;

        //TAUAddrTrie
        TAUAddrTrie::Trie trie;
        for(int i = 0; i < test2Num; i++)
            trie.Insert(addresses[i]);

        start = clock();
        for(size_t i = 0; i < addressesRandom.size(); i++)
        {
            if (trie.Search(addressesRandom[i]))
                trie.Remove(addressesRandom[i]);
        }
        finish = clock();
        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
        cout<<"trie_"<<addressesRandom.size()<<" 的read_randomly时间为"<<totaltime<<"秒！"<<endl;
    }

//    // test 3, modify randomly: set map list vector TAUAddrTrie
//    int test3Num = num;
//    cout<<"modify_randomly test"<<endl;
//    vector<string> addressesInsert;
//    for(size_t i = 0; i < addressesRandom.size(); i++)
//        addressesInsert.push_back(GetRandomAddress());
//    for(int t = 0; t < 1; t++, test3Num*=10)
//    {
//        //set
//        set<string> s;
//        for(int i = 0; i < test3Num; i++)
//            s.insert(addresses[i]);

//        std::clock_t start, finish;
//        start = clock();
//        set<string>::iterator ite;
//        for(size_t i = 0; i < addressesRandom.size(); i++)
//        {
//            s.erase(addressesRandom[i]);
//            s.insert(addressesInsert[i]);
//        }
//        finish = clock();
//        double totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
//        cout<<"set_"<<addressesRandom.size()<<" 的modify_randomly时间为"<<totaltime<<"秒！"<<endl;

//        //map
//        map<string, string> m;
//        for(int i = 0; i < test3Num; i++)
//            m.insert(pair<string, string>(addresses[i], addresses[i]));

//        start = clock();
//        for(size_t i = 0; i < addressesRandom.size(); i++)
//            m[addressesRandom[i]] = addressesInsert[i];
//        finish = clock();
//        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
//        cout<<"map_"<<addressesRandom.size()<<" 的modify_randomly时间为"<<totaltime<<"秒！"<<endl;

//        //list
//        list<string> l;
//        for(int i = 0; i < test3Num; i++)
//            l.push_back(addresses[i]);

//        start = clock();
//        for(size_t i = 0; i < addressesRandom.size(); i++)
//        {
//            list<string>::iterator ite = find(l.begin(), l.end(), addressesRandom[i]);
//            l.erase(ite);
//            l.push_back(addressesInsert[i]);
//        }
//        finish = clock();
//        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
//        cout<<"list_"<<addressesRandom.size()<<" 的modify_randomly时间为"<<totaltime<<"秒！"<<endl;

//        //vector
//        vector<string> v;
//        for(int i = 0; i < test3Num; i++)
//            v.push_back(addresses[i]);

//        start = clock();
//        for(size_t i = 0; i < addressesRandom.size(); i++)
//        {
//            vector<string>::iterator ite = find(v.begin(), v.end(), addressesRandom[i]);
//            v.erase(ite);
//            v.push_back(addressesInsert[i]);
//        }
//        finish = clock();
//        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
//        cout<<"vector_"<<addressesRandom.size()<<" 的modify_randomly时间为"<<totaltime<<"秒！"<<endl;

//        //TAUAddrTrie
//        TAUAddrTrie::Trie trie;
//        for(int i = 0; i < test3Num; i++)
//            trie.Insert(addresses[i]);

//        start = clock();
//        for(size_t i = 0; i < addressesRandom.size(); i++)
//        {
//            if (trie.Search(addressesRandom[i]))
//            {
//                trie.Remove(addressesRandom[i]);
//                trie.Insert(addressesInsert[i]);
//            }
//        }
//        finish = clock();
//        totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
//        cout<<"trie_"<<addressesRandom.size()<<" 的modify_randomly时间为"<<totaltime<<"秒！"<<endl;
//    }

}

BOOST_AUTO_TEST_SUITE_END()
