// Copyright (c) 2018- The ISONO Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ISONO_TX_UTILS_H
#define ISONO_TX_UTILS_H

#include "primitives/transaction.h"

class CTransactionUtils
{
    public:
        static bool SendTransaction(const CMutableTransaction& mutx, const CAmount fee, uint256& hashTx, std::string& failReason);
        static bool CreateTransaction(std::map<std::string, CAmount>& receipts, const std::string& pubKey,
                const std::string& prvKey, CFeeRate& userFee, CMutableTransaction& tx, CAmount& nFeeRet, std::string& strFailReason);

    private:
        CTransactionUtils() {}

        static bool SelectCoinsMinConf(const CAmount& nTargetValue, int nConf, std::vector<COutPoint>& vCoins,
                std::set<COutPoint>& setCoinsRet, CAmount& nValueRet);

        static bool SelectCoins(std::vector<COutPoint>& vAvailableCoins, const CAmount& nTargetValue, std::set<COutPoint>& setCoinsRet, CAmount& nValueRet);

        static bool AvailableCoins(const std::string& pubKey, std::vector<COutPoint>& vCoins);

        static bool SelectRewards(std::vector<CTxReward>& vAvailableRewards, const CAmount& nTargetValue, std::vector<CTxReward>& setRewardsRet, CAmount& nValueRet);

        static bool AvailableRewards(const std::string& pubKey, std::vector<CTxReward>& vRewards);
};

#endif //ISONO_TX_UTILS_H