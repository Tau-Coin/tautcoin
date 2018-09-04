// Copyright (c) 2018- The taucoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rewardman.h"

#include "main.h"
#include "util.h"
#include "sync.h"

extern CMemberInfoDB *pmemberinfodb;

extern bool ConvertPubkeyToAddress(const std::string& pubKey, std::string& addrStr); 

static CCriticalSection cs_rwdman;

RewardManager* RewardManager::pSingleton = NULL;

RewardManager* RewardManager::GetInstance()
{
    LOCK(cs_rwdman);

    if (pSingleton == NULL)
    {
        pSingleton = new RewardManager();
    }

    return pSingleton;
}

CAmount RewardManager::GetRewardsByAddress(std::string& address, int height)
{
    if (height <= -1)
    {
        LOCK(cs_main);
        height = chainActive.Height();
    }
    return pmemberinfodb->GetRwdBalance(address, height);
}

CAmount RewardManager::GetRewardsByPubkey(const std::string &pubkey, int height)
{
    std::string addrStr;

    if (!ConvertPubkeyToAddress(pubkey, addrStr))
        return 0;

    return GetRewardsByAddress(addrStr, height);
}

RewardManager::RewardManager(){}
