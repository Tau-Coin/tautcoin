// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TAUCOIN_REWARD_MANAGER_H
#define TAUCOIN_REWARD_MANAGER_H

#include "amount.h"
//#include "isndb.h"

class RewardManager
{
public:

    int currentHeight;// use to update reward to avoid repetition

    static RewardManager* GetInstance();

    CAmount GetRewardsByAddress(std::string& addr, int height = -1);

    bool UpdateRewardsByAddress(std::string& address, CAmount newRewards, CAmount oldReward);

    CAmount GetRewardsByPubkey(const std::string &pubkey, int height = -1);

    bool UpdateRewardsByPubkey(const std::string &pubkey, CAmount newRewards, CAmount oldReward);

    bool GetMembersByClubID(uint64_t clubID, std::vector<std::string> &addresses, std::string& leaderAddr);

    bool GetMembersTxCountByClubID(uint64_t clubID, std::map<std::string, uint64_t>& addrToTC, std::string& leaderAddr);

    uint64_t GetTxCountByAddress(std::string& address, int height = -1);

protected:

    RewardManager();

private:

    static RewardManager* pSingleton;

    //ISNDB* backendDb;
};
#endif // TAUCOIN_REWARD_MANAGER_H
