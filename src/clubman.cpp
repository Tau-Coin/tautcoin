// Copyright (c) 2018- The taucoin Core developers
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

//uint64_t ClubManager::GetHarvestPowerByAddress(std::string& address, int nHeight)
//{
//    if (nHeight <= -1)
//    {
//        LOCK(cs_main);
//        nHeight = chainActive.Height();
//    }
//    return paddrinfodb->GetHarvestPowerByAddress(address, nHeight);
//}

ClubManager::ClubManager() {}

//bool ClubManager::IsAllowForge(const std::string& pubKey, int nHeight, uint64_t &harvestPower)
//{
//    if (pubKey.empty() || nHeight < 0)
//    {
//        return false;
//    }

//    std::string addrStr;
//    if (!ConvertPubkeyToAddress(pubKey, addrStr))
//    {
//        return false;
//    }

//    harvestPower = GetHarvestPowerByAddress(addrStr, nHeight);
//    if (harvestPower > DEFAULT_HARVEST_POWER)
//    {
//        return true;
//    }

//    return false;
//}

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

    uint64_t power = paddrinfodb->GetHarvestPowerByAddress(strAddr, nHeight);
    memCount = power;
    addr.SetString(strAddr);

    if (power > DEFAULT_HARVEST_POWER)
    {
        return true;
    }

    return false;
}
