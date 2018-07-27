// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clubman.h"

#include "util.h"
#include "sync.h"

extern bool ConvertPubkeyToAddress(const std::string& pubKey, std::string& addrStr);

static CCriticalSection cs_clubman;

ClubManager* ClubManager::pSingleton = NULL;

ClubManager* ClubManager::GetInstance()
{
    LOCK(cs_clubman);

    if (pSingleton == NULL)
    {
        pSingleton = new ClubManager();
    }

    return pSingleton;
}

uint64_t ClubManager::GetHarvestPowerByAddress(std::string& address, int nHeight)
{
    nHeight = Params().GetConsensus().PodsAheadTargetHeight(nHeight);

    std::vector<string> fields;
	fields.push_back(clubFieldCount);
	mysqlpp::StoreQueryResult bLocal = backendDb->ISNSqlSelectAA(tableClub, fields, clubFieldAddress, address);

    LogPrintf("%s, %s, %d\n", __func__, address, bLocal.num_rows());
    if (bLocal.num_rows() > 0)
    {
	    return (uint64_t)bLocal[0]["ttc"];
    }
    else
    {
        return DEFAULT_HARVEST_POWER;
    }
}

ClubManager::ClubManager()
{
    backendDb = ISNDB::GetInstance();
}

bool ClubManager::IsAllowForge(const std::string& pubKey, int nHeight)
{
    if (pubKey.empty() || nHeight < 0)
    {
        return false;
    }

    std::string addrStr;
    if (!ConvertPubkeyToAddress(pubKey, addrStr))
    {
        return false;
    }

    uint64_t power = GetHarvestPowerByAddress(addrStr, nHeight);
    if (power > DEFAULT_HARVEST_POWER)
    {
        return true;
    }

    return false;
}

bool ClubManager::IsForgeScript(const CScript& script, CBitcoinAddress& addr, uint64_t& memCount) {
    int nHeight = 0;
    {
        LOCK(cs_main);
        nHeight = chainActive.Height();
    }
    nHeight = Params().GetConsensus().PodsAheadTargetHeight(nHeight);

    std::string strAddr;
    if (!addr.ScriptPub2Addr(script, strAddr)) {
        LogPrintf("isForgeScript, ScriptPub2Addr fail\n");
        return false;
    }

    uint64_t power = GetHarvestPowerByAddress(strAddr, nHeight);
    memCount = power;
    addr.SetString(strAddr);

    if (power > DEFAULT_HARVEST_POWER)
    {
        return true;
    }

    return false;
}

bool ClubManager::GetClubIDByAddress(const std::string& address, uint64_t& clubID)
{
    std::vector<string> fields;
	fields.push_back(clubFieldID);
	mysqlpp::StoreQueryResult bLocal = backendDb->ISNSqlSelectAA(tableClub, fields, clubFieldAddress, address);

    LogPrintf("%s, %s, %d\n", __func__, address, bLocal.num_rows());
    if (bLocal.num_rows() > 0)
    {
	    clubID = (uint64_t)bLocal[0]["club_id"];
        return true;
    }
    else
    {
        return false;
    }
}

bool ClubManager::GetClubIDByPubkey(const std::string& pubkey, uint64_t& clubID)
{
    if (pubkey.empty())
    {
        return false;
    }

    std::string addrStr;
    if (!ConvertPubkeyToAddress(pubkey, addrStr))
    {
        return false;
    }

    return GetClubIDByAddress(addrStr, clubID);
}

