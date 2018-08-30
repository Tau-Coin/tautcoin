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

#include <boost/assign/list_of.hpp>
#include <boost/shared_ptr.hpp>

#include <univalue.h>

//#define tautest    //it is a debug function switch,if you does not want please turn off it
using namespace std;

std::string static EncodeDumpTime(int64_t nTime) {
    return DateTimeStrFormat("%Y-%m-%dT%H:%M:%SZ", nTime);
}

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
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "dumpclubmembers \"filename\"\n"
            "\nDumps all mining clubs and members in a human-readable format.\n"
            "\nArguments:\n"
            "1. \"filename\"    (string, required) The filename\n"
            "\nExamples:\n"
            + HelpExampleCli("dumpclubmembers", "\"test\"")
            + HelpExampleRpc("dumpclubmembers", "\"test\"")
        );

    std::ofstream file;
    file.open(params[0].get_str().c_str());
    if (!file.is_open())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open dump file");

    int height = 0;
    {
        LOCK(cs_main);
        height = chainActive.Height();
    }

    // produce output
    file << strprintf("Height %i ,\n", chainActive.Height());

    std::vector<std::string> leaders;

    bool result = pclubinfodb->GetAllClubLeaders(leaders);
    if (!result)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Get club leaders failed");

    // Output clubinfo
    file << "\n";
    file << "club leaders count:" << leaders.size() << "\n";
    file << "index \t address \t\t mining power" << "\n";

    int i = 1;
    uint64_t totalTTC = 0;
    for (std::vector<std::string>::iterator it = leaders.begin();
        it != leaders.end(); it++, i++)
    {
        std::string address = *it;
        uint64_t ttc = pmemberinfodb->GetTotalTXCnt(address, height);
        totalTTC += ttc;
        file << i + 1 << "\t" << address << "\t" << ttc << "\n";
    }

    file.close();
    return NullUniValue;
}


static const CRPCCommand commands[] =
{ //  category              name                        actor (function)           okSafeMode
  //  --------------------- ------------------------    -----------------------    ----------
    { "clubmember",         "getminingpowerbyaddress",  &getminingpowerbyaddress,  true  },
    { "clubmember",         "dumpclubmembers",          &dumpclubmembers,          true  },
};

void RegisterClubmemberRPCCommands(CRPCTable &tableRPC)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}

