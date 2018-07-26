// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ISNCOIN_REWARD_MANAGER_H
#define ISNCOIN_REWARD_MANAGER_H

#include "amount.h"
#include "isndb.h"

class RewardManager
{
    public:

        static RewardManager* GetInstance();

        CAmount GetRewardsByAddress(std::string& addr);

        CAmount GetRewardsByPubkey(const std::string &pubkey);

    protected:

        RewardManager();

    private:

        static RewardManager* pSingleton;

        ISNDB* backendDb;
};
#endif // ISNCOIN_REWARD_MANAGER_H
