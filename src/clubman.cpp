// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clubman.h"

#include "main.h"
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
    /*
    std::vector<string> fields;
	fields.push_back(clubFieldCount);
	mysqlpp::StoreQueryResult bLocal = backendDb->ISNSqlSelectAA(tableClub, fields, clubFieldAddress, address);

    if (bLocal.num_rows() > 0)
    {
        LogPrint("club", "%s, %s, %d\n", __func__, address, (uint64_t)bLocal[0]["ttc"]);
	    return (uint64_t)bLocal[0]["ttc"];
    }
    else
    {
        return DEFAULT_HARVEST_POWER;
    }
    */
    if (nHeight <= -1)
    {
        LOCK(cs_main);
        nHeight = chainActive.Height();
    }
    return pmemberinfodb->GetHarvestPowerByAddress(address, nHeight);
}

ClubManager::ClubManager()
{
    backendDb = ISNDB::GetInstance();
}

bool ClubManager::IsAllowForge(const std::string& pubKey, int nHeight, uint64_t &harvestPower)
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

    harvestPower = GetHarvestPowerByAddress(addrStr, nHeight);
    if (harvestPower > DEFAULT_HARVEST_POWER)
    {
        return true;
    }

    return false;
}

bool ClubManager::IsForgeScript(const CScript& script, CBitcoinAddress& addr, uint64_t& memCount, int nHeight) {
    if (nHeight <= -1)
    {
        LOCK(cs_main);
        nHeight = chainActive.Height();
    }

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

    if (bLocal.num_rows() > 0)
    {
	    clubID = (uint64_t)bLocal[0]["club_id"];
        LogPrint("club", "%s, %s, %d\n", __func__, address, clubID);
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
