// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TAUCOIN_CLUB_MANAGER_H
#define TAUCOIN_CLUB_MANAGER_H

#include "amount.h"
#include "isndb.h"

class ClubManager
{
    public:

        static const int DEFAULT_HARVEST_POWER = 1;

        static ClubManager* GetInstance();

        uint64_t GetHarvestPowerByAddress(std::string& addr);

    protected:

        ClubManager();

    private:

        static ClubManager* pSingleton;

        ISNDB* backendDb;
};
#endif // TAUCOIN_REWARD_MANAGER_H

