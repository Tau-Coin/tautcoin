// Copyright (c) 2018- The Taucoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "amount.h"
#include "chain.h"
#include "chainparams.h"
#include "clubman.h"
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
    
    uint64_t miningpower = pmemberinfodb->GetHarvestPowerByAddress(addrStr, height);

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("address", addrStr));
    result.push_back(Pair("miningpower", miningpower));

    return result;
}

UniValue dumpclubmembers(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "dumpclubmembers \"filename\" height\n"
            "\nDumps all mining clubs and members in a human-readable format.\n"
            "\nArguments:\n"
            "1. \"filename\"    (string, required) The filename\n"
            "2. height          (numberic, optional) The height\n"
            "\nExamples:\n"
            + HelpExampleCli("dumpclubmembers", "\"test\" 1000")
            + HelpExampleRpc("dumpclubmembers", "\"test\" 1000")
        );

    RPCTypeCheck(params, boost::assign::list_of(UniValue::VSTR)(UniValue::VNUM), true);

    std::ofstream file;
    file.open(params[0].get_str().c_str());
    if (!file.is_open())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open dump file");

    int height = 0;
    int chainHeight;
    {
        LOCK(cs_main);
        chainHeight = chainActive.Height();
    }
    if (params.size() == 1)
    {
        LOCK(cs_main);
        height = chainHeight;
    }
    else if (params.size() == 2)
    {
        height = params[1].get_int();
    }

    if (height < 0 || height > chainHeight)
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid height");
    }

    // produce output
    file << strprintf("Height %i \n", height);

    std::vector<std::string> leaders;

    bool result = pclubinfodb->GetAllClubLeaders(leaders, height);
    if (!result)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Get club leaders failed");

    // Output clubinfo
    file << "club information table:\n";
    file << "\n";
    file << "club leaders count:" << leaders.size() << "\n";
    file << "index \t address \t\t\t\t mining power" << "\n";

    int i = 0;
    uint64_t totalTTC = 0;
    for (std::vector<std::string>::iterator it = leaders.begin();
        it != leaders.end(); it++, i++)
    {
        std::string address = *it;
        uint64_t ttc = 0;//pmemberinfodb->GetTotalTXCnt(address, height);
        std::string dummyStr;
        uint64_t dummyInt;
        CAmount  dummyValue;
        pmemberinfodb->GetFullRecord(address, height, dummyStr, dummyStr, dummyInt, ttc, dummyValue, true);
        totalTTC += ttc;
        file << i + 1 << "\t\t" << address << "\t" << ttc << "\n";
    }
    file << "\n";
    file << "Total TTC:" << totalTTC  << "\n";
    file << "\n\n\n";

    file << "member information table:\n";
    file << "\n";
    file << "\n";

    i = 1;
    uint64_t totalTC = 0;
    uint64_t totalRewards = 0;
    for (std::vector<std::string>::iterator it = leaders.begin();
        it != leaders.end(); it++, i++)
    {
        uint64_t  itTC = 0;
        uint64_t  itRewards = 0;
        std::string address = *it;
        uint64_t ttcleader = 0;//pmemberinfodb->GetTotalTXCnt(address, height);
        std::string dummyStr;
        uint64_t dummyInt;
        CAmount  dummyValue;
        pmemberinfodb->GetFullRecord(address, height, dummyStr, dummyStr, dummyInt, ttcleader, dummyValue, true);
        file << i << " club:" << address << ", ttc:" << ttcleader << "\n";
        file << "\t" << "index\t" << "address\t\t\t\t\t\t\t\t" << "packer\t" << "father\t"
             << "tc\t" << "reward" << "\n";

        // Firstly, output leader info
        std::string packer;
        std::string father;
        uint64_t    tc = 0;
        uint64_t    ttc = 0;
        CAmount     value = 0;

        int j = 1;

        pmemberinfodb->GetFullRecord(address, height, packer, father, tc, ttc, value, true);
        file << "\t" << j <<"\t\t" << address <<"\t" << packer <<"\t\t" << father << "\t\t"
             << tc << "\t\t" << value << "\n";
        itTC += tc;
        itRewards += value;
        j++;

        std::vector<std::string> members
            = pclubinfodb->GetTotalMembersByAddress(address, height, true);
        for (std::vector<std::string>::iterator itr = members.begin();
            itr != members.end(); itr++, j++)
        {
            pmemberinfodb->GetFullRecord(*itr, height, packer, father, tc, ttc, value, true);
            file << "\t" << j <<"\t\t" << *itr <<"\t" << packer <<"\t\t" << father << "\t\t"
                << tc << "\t\t" << value << "\n";
            itTC += tc;
            itRewards += value;
        }

        file << "\n\n\ttotalTC:" << itTC
            << ", totalRewards:" << itRewards << "\n";

        totalTC += itTC;
        totalRewards += itRewards;

        file << "\n\n";
    }

    file << "\n\n";
    file << "Total TC:" << totalTC << ", total rewards:" << totalRewards << "\n";

    file.close();

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("Result", "finished"));
    return ret;
}

UniValue getrewardrate(const UniValue& params, bool fHelp)
{
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
            "\nGet  reward rate at height 1000\n"
            + HelpExampleCli("getrewardrate", "1000")
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
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getmemberinfo address height\n"
            "\nGet member information by address and chain height.\n"
            "\nArguments:\n"
            "1. address    (string, required) The address to get member info.\n"
            "2. height     (numeric, optinal) the blockchain height.\n"
            "\nResult\n"
            "{\n"
            "    \"height\": <height>\n"
            "    \"address\": <addr>\n"
            "    \"clubleader\": <mining club leader>\n"
            "    \"father\": <address father>\n"
            "    \"miningpower\": <address's miningpower>\n"
            "    \"rewards\": <address's rewards>\n"
            "}\n"
            "\nExamples:\n"
            "\nGet member info by address at height 1000\n"
            + HelpExampleCli("getmemberinfo", "\"myaddress\" 1000")
        );

    RPCTypeCheck(params, boost::assign::list_of(UniValue::VSTR)(UniValue::VNUM), true);

    int chainHeight = 0;
    {
        LOCK(cs_main);
        chainHeight = chainActive.Height();
    }

    std::string addrStr = params[0].get_str();

    int height = 0;
    if (params.size() == 1)
    {
        height = chainHeight;
    }
    else if (params.size() == 2)
    {
        height = params[1].get_int();
        if (height < 0 || height > chainHeight)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Error: Invalid height");
    }

    CBitcoinAddress address(addrStr);
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Error: Invalid address");

    std::string leader;
    std::string father;
    uint64_t selfMP;
    uint64_t clubMP;
    CAmount rewards;
    pmemberinfodb->GetFullRecord(addrStr, height, leader, father, selfMP, clubMP, rewards, true);

    // In this case, addrStr is a mining club leader
    if (leader.compare("0") == 0 && father.compare("0") == 0)
    {
        leader = addrStr;
        father = addrStr;
    }
    else
    {
        clubMP = pmemberinfodb->GetHarvestPowerByAddress(leader, height);
    }

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("address", addrStr));
    result.push_back(Pair("height", height));
    result.push_back(Pair("clubleader", leader));
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

