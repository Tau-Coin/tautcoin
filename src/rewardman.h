// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TAUCOIN_REWARD_MANAGER_H
#define TAUCOIN_REWARD_MANAGER_H

#include "amount.h"
#include "isndb.h"

class RewardManager
{
public:

    int currentHeight;// use to update reward to avoid repetition

    static RewardManager* GetInstance();

    CAmount GetRewardsByAddress(std::string& addr);

    bool UpdateRewardsByAddress(std::string& address, CAmount rewards);

    CAmount GetRewardsByPubkey(const std::string &pubkey);

    bool UpdateRewardsByPubkey(const std::string &pubkey, CAmount rewards);

    bool GetMembersByClubID(uint64_t clubID, std::vector<string> &addresses);

protected:

    RewardManager();

private:

    static RewardManager* pSingleton;

    ISNDB* backendDb;
};
#endif // TAUCOIN_REWARD_MANAGER_H
