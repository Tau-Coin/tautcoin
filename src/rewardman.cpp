// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rewardman.h"

#include "util.h"
#include "sync.h"
//#include "tool.h"

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

CAmount RewardManager::GetRewardsByAddress(std::string& address)
{
	std::vector<string> fields;
	fields.push_back(memFieldBalance);
	mysqlpp::StoreQueryResult bLocal = backendDb->ISNSqlSelectAA(tableMember, fields, memFieldAddress, address);
	return bLocal[0]["balance"];
}

CAmount RewardManager::GetRewardsByPubkey(const std::string &pubkey)
{
    std::string addrStr;

    if (!ConvertPubkeyToAddress(pubkey, addrStr))
        return 0;

    return 10*COIN;//getBalanceByAddress(addrStr);
}

RewardManager::RewardManager()
{
    backendDb = ISNDB::GetInstance();
}
