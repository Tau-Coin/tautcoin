// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clubman.h"

#include "util.h"
#include "sync.h"

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

uint64_t ClubManager::GetHarvestPowerByAddress(std::string& address)
{
    return 0;

    std::vector<string> fields;
	fields.push_back(clubFieldCount);
	mysqlpp::StoreQueryResult bLocal = backendDb->ISNSqlSelectAA(tableClub, fields, clubFieldAddress, address);
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

