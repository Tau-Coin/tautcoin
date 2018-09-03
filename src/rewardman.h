// Copyright (c) 2018- The taucoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TAUCOIN_REWARD_MANAGER_H
#define TAUCOIN_REWARD_MANAGER_H

#include "amount.h"

class RewardManager
{
public:

    int currentHeight;// use to update reward to avoid repetition

    static RewardManager* GetInstance();

    CAmount GetRewardsByAddress(std::string& addr, int height = -1);

    CAmount GetRewardsByPubkey(const std::string &pubkey, int height = -1);

protected:

    RewardManager();

private:

    static RewardManager* pSingleton;
};
#endif // TAUCOIN_REWARD_MANAGER_H
