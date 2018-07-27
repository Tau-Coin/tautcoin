// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TAUCOIN_CLUB_MANAGER_H
#define TAUCOIN_CLUB_MANAGER_H

#include "amount.h"
#include "base58.h"
#include "consensus/params.h"
#include "isndb.h"
#include "main.h"
#include "script/script.h"

class ClubManager
{
    public:

        static const int DEFAULT_HARVEST_POWER = 1;

        static ClubManager* GetInstance();

        uint64_t GetHarvestPowerByAddress(std::string& addr, int nHeight);

        bool IsAllowForge(const std::string& pubKey, int nHeight);

        bool IsForgeScript(const CScript& script, CBitcoinAddress& addr, uint64_t& memCount);

        bool GetClubIDByAddress(const std::string& address, uint64_t& clubID);

        bool GetClubIDByPubkey(const std::string& pubkey, uint64_t& clubID);

    protected:

        ClubManager();

    private:

        static ClubManager* pSingleton;

        ISNDB* backendDb;
};
#endif // TAUCOIN_REWARD_MANAGER_H

