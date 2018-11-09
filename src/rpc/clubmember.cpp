// Copyright (c) 2018- The Taucoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "amount.h"
#include "chain.h"
#include "chainparams.h"
#include "consensus/consensus.h"
#include "consensus/params.h"
#include "consensus/validation.h"
#include "core_io.h"
#include "init.h"
#include "main.h"
#include "consensus/merkle.h"
#include "miner.h"
#include "net.h"
#include "pot.h"
#include "rpc/server.h"
#include "timedata.h"
#include "tool.h"
#include "txmempool.h"
#include "util.h"
#include "utilstrencodings.h"
#include "validationinterface.h"
#include "wallet/wallet.h"

#include <stdint.h>
#include <fstream>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/shared_ptr.hpp>

#include <univalue.h>

//#define tautest    //it is a debug function switch,if you does not want please turn off it
using namespace std;

UniValue getminingpowerbyaddress(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "getminingpowerbyaddress address height\n"
            "\nGet mining power by address from database.\n"
            "\nArguments:\n"
            "1. address    (string, required) The address to get mining power.\n"
            "2. height     (numeric, required) the blockchain height.\n"
            "\nResult\n"
            "{\n"
            "    \"address\": <addr>\n"
            "    \"miningpower\": <power>\n"
            "}\n"
            "\nExamples:\n"
            "\nGet myaddress mining power at height 1000\n"
            + HelpExampleCli("getminingpowerbyaddress", "\"myaddress\" 1000")
        );

    RPCTypeCheck(params, boost::assign::list_of(UniValue::VSTR)(UniValue::VNUM), true);

    int chainHeight = 0;
    {
        LOCK(cs_main);
        chainHeight = chainActive.Height();
    }

    std::string addrStr = params[0].get_str();
    int height = params[1].get_int();

    if (height < 0 || height > chainHeight)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Error: Invalid height");

    CBitcoinAddress address(addrStr);

    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Error: Invalid address");
    
    uint64_t miningpower = paddrinfodb->GetHarvestPowerByAddress(addrStr, height);

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("address", addrStr));
    result.push_back(Pair("miningpower", miningpower));

    return result;
}

UniValue dumpclubmembers(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "dumpclubmembers \\\"filename\\\"\n"
            "\nDumps all mining clubs and members in a readable format.\n"
            "\nArguments:\n"
            "1. \"filename\"    (string, required) The filename\n"
            "\nExamples:\n"
            + HelpExampleCli("dumpclubmembers", "\"test\"")
            + HelpExampleRpc("dumpclubmembers", "\"test\"")
        );

    RPCTypeCheck(params, boost::assign::list_of(UniValue::VSTR)(UniValue::VNUM), true);

    std::ofstream file;
    file.open(params[0].get_str().c_str());
    if (!file.is_open())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open dump file");

    int height = 0;
    LOCK(cs_main);
    height = chainActive.Height();

    // produce output
    file << strprintf("Height %i \n", height);

    std::vector<std::string> miners;
    miners = paddrinfodb->GetAllClubMiners();

    // Output clubinfo
    file << "club information table:\n";
    file << "\n";
    file << "club miners count:" << miners.size() << "\n";
    file << "index \t\t address \t\t\t\tmining power(MP)" << "\n";

    int i = 0;
    uint64_t allTotalMP = 0;
    for (std::vector<std::string>::iterator it = miners.begin();
        it != miners.end(); it++, i++)
    {
        std::string address = *it;
        CTAUAddrInfo addrInfo = paddrinfodb->GetAddrInfo(address, height);
        uint64_t totalMP = addrInfo.totalMP;

        allTotalMP += totalMP;
        file << i + 1 << "\t\t" << address << "\t" << totalMP << "\n";
    }
    file << "\n";
    file << "All totalMP:" << allTotalMP  << "\n";
    file << "\n\n\n";

    file << "member information table:\n";
    file << "\n";
    file << "\n";

    i = 1;
    uint64_t allTotalMP_2 = 0;
    uint64_t totalRewards = 0;
    for (std::vector<std::string>::iterator it = miners.begin();
        it != miners.end(); it++, i++)
    {
        uint64_t  iTotalMP = 0;
        uint64_t  itRewards = 0;
        std::string address = *it;
        CTAUAddrInfo addrInfo = paddrinfodb->GetAddrInfo(address, height);
        uint64_t totalMP = addrInfo.totalMP;
        file << i << " club:" << address << ", totalMP:" << totalMP << "\n";
        file << "\t" << "index\t\t" << "address\t\t\t\t\t\t" << "miner\t\t\t\t\t\t" << "father\t\t\t\t\t"
             << "MP\t\t   " << "reward" << "\n";

        // Firstly, output miners info
        //addrInfo = paddrinfodb->GetAddrInfo(address, height);
        uint64_t index = addrInfo.index;
        std::string miner = (addrInfo.miner.compare("0") == 0) ? address : addrInfo.miner;
        std::string father = (addrInfo.father.compare("0") == 0) ? address : addrInfo.father;
        uint64_t MP = pclubinfodb->GetCacheRecord(father)[index].MP;
        CAmount value = pclubinfodb->GetCacheRecord(father)[index].rwd;

        int j = 1;

        file << "\t" << j <<"\t\t" << address <<"\t" << miner <<"\t\t" << father << "\t\t"
             << MP << "\t\t" << value << "\n";
        iTotalMP += MP;
        itRewards += value;
        j++;

        // Secondly, output members info
        std::vector<std::string> members;
        pclubinfodb->GetTotalMembersByAddress(address, members);
        std::string memberFather;
        for (std::vector<std::string>::iterator itr = members.begin();
            itr != members.end(); itr++, j++)
        {
            addrInfo = paddrinfodb->GetAddrInfo(*itr, height);
            index = addrInfo.index;
            memberFather = (addrInfo.father.compare("0") == 0) ? *itr : addrInfo.father;
            file << "\t" << j <<"\t\t" << *itr <<"\t" << addrInfo.miner <<"\t\t" << addrInfo.father << "\t\t"
                << pclubinfodb->GetCacheRecord(memberFather)[index].MP << "\t\t"
                << pclubinfodb->GetCacheRecord(memberFather)[index].rwd << "\n";
            iTotalMP += pclubinfodb->GetCacheRecord(memberFather)[index].MP;
            itRewards += pclubinfodb->GetCacheRecord(memberFather)[index].rwd;
        }

        file << "\n\n\ttotalMP:" << iTotalMP
            << ", totalRewards:" << itRewards << "\n";

        allTotalMP_2 += iTotalMP;
        totalRewards += itRewards;

        file << "\n\n";
    }

    file << "\n\n";
    file << "All totalMP:" << allTotalMP_2 << ", total rewards:" << totalRewards << "\n";

    file.close();

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("Result", "finished"));
    return ret;
}

UniValue getrewardrate(const UniValue& params, bool fHelp)
{
    bool updateRewardRate = false;
    if (mapArgs.count("-updaterewardrate") && mapMultiArgs["-updaterewardrate"].size() > 0)
    {
        string flag = mapMultiArgs["-updaterewardrate"][0];
        if (flag.compare("true") == 0)
            updateRewardRate = true;
    }
    if (!updateRewardRate)
    {
        throw runtime_error(
                    "need by \"-updaterewardrate=true\" parameter\n"
                    );
    }

    if (fHelp || params.size() > 2)
        throw runtime_error(
            "getrewardrate height\n"
            "\nGet reward rate by chain height.\n"
            "\nArguments:\n"
            "1. height     (numeric, required) the blockchain height.\n"
            "\nResult\n"
            "{\n"
            "    \"height\": <height>\n"
            "    \"address\": <addr>\n"
            "    \"rewardrate\": <rewardrate>\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getrewardrate", "1000")
            + HelpExampleRpc("getrewardrate", "1000")
        );

    RPCTypeCheck(params, boost::assign::list_of(UniValue::VNUM), true);

    int chainHeight = 0;
    {
        LOCK(cs_main);
        chainHeight = chainActive.Height();
    }

    int height = 0;
    if (params.size() == 0)
    {
        height = chainHeight;
    }
    else
    {
        height = params[0].get_int();
        if (height < 0 || height > chainHeight)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Error: Invalid height");
    }

    static CRewardRateViewDB *pRewardRateView = NULL;
    if (pRewardRateView == NULL)
    {
        pRewardRateView = pclubinfodb->GetRewardRateDBPointer();
    }

    std::string addr_rate;
    if (!pRewardRateView->GetRewardRate(height, addr_rate))
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Error: get reward rate fail");
    }

    std::vector<string> fields;
    boost::split(fields, addr_rate, boost::is_any_of("_"));
    if (fields.size() != 2)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Error: incorrect reward rate data");

    std::string address = fields[0];
    double rate = 0;
    stringstream stream(fields[1]);
    stream >> rate;

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("height", height));
    result.push_back(Pair("address", address));
    result.push_back(Pair("rewardrate", rate));

    return result;
}

UniValue getmemberinfo(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getmemberinfo \\\"address\\\"\n"
            "\nGet member information by address.\n"
            "\nArguments:\n"
            "1. address    (string, required) The address to get member info.\n"
            "\nResult\n"
            "{\n"
            "    \"height\": <height>\n"
            "    \"address\": <addr>\n"
            "    \"clubminer\": <mining club miner>\n"
            "    \"father\": <address father>\n"
            "    \"miningpower\": <address's miningpower>\n"
            "    \"rewards\": <address's rewards>\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getmemberinfo", "\"myaddress\"")
            + HelpExampleRpc("getmemberinfo", "\"myaddress\"")
        );

    RPCTypeCheck(params, boost::assign::list_of(UniValue::VSTR)(UniValue::VNUM), true);

    std::string addrStr = params[0].get_str();

    LOCK(cs_main);
    int height = chainActive.Height();

    CBitcoinAddress address(addrStr);
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Error: Invalid address");

    CTAUAddrInfo addrInfo = paddrinfodb->GetAddrInfo(addrStr, height);
    uint64_t index = addrInfo.index;
    std::string miner = (addrInfo.miner.compare("0") == 0) ? addrStr : addrInfo.miner;
    std::string father = (addrInfo.father.compare("0") == 0) ? addrStr : addrInfo.father;
    std::vector<CMemberInfo> vc = pclubinfodb->GetCacheRecord(father);
    if(vc.empty()||index >=vc.size()){
       throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Error: Invalid address,index out of range");
    }
    uint64_t selfMP = vc[index].MP;
    uint64_t clubMP = addrInfo.totalMP;
    CAmount rewards = vc[index].rwd;

    clubMP = paddrinfodb->GetHarvestPowerByAddress(miner, height);

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("address", addrStr));
    result.push_back(Pair("height", height));
    result.push_back(Pair("clubminer", miner));
    result.push_back(Pair("clubpower", clubMP));
    result.push_back(Pair("selfpower", selfMP));
    result.push_back(Pair("father", father));
    result.push_back(Pair("rewards", rewards));

    return result;
}


static const CRPCCommand commands[] =
{ //  category              name                        actor (function)           okSafeMode
  //  --------------------- ------------------------    -----------------------    ----------
    { "clubmember",         "getminingpowerbyaddress",  &getminingpowerbyaddress,  true  },
    { "clubmember",         "dumpclubmembers",          &dumpclubmembers,          true  },
    { "clubmember",         "getrewardrate",            &getrewardrate,            true  },
    { "clubmember",         "getmemberinfo",            &getmemberinfo,            true  },
};

void RegisterClubmemberRPCCommands(CRPCTable &tableRPC)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}

