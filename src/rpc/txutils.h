// Copyright (c) 2018- The Taucoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TAUCOIN_TX_UTILS_H
#define TAUCOIN_TX_UTILS_H

#include "coins.h"
#include "primitives/transaction.h"
#include "txmempool.h"

class CTransactionUtils
{
    public:
        static bool SendTransaction(const CMutableTransaction& mutx, const CAmount fee, uint256& hashTx, std::string& failReason);
        static bool CreateTransaction(std::map<std::string, CAmount>& receipts, const bool bSubtractFeeFromReceipts, const std::string& pubKey,
                const std::string& prvKey, CFeeRate& userFee, CMutableTransaction& tx, CAmount& nFeeRet, std::string& strFailReason);

    private:
        CTransactionUtils() {}

        static bool SelectCoinsMinConf(const CAmount& nTargetValue, int nConf, std::vector<COutPoint>& vCoins,
                std::set<COutPoint>& setCoinsRet, CAmount& nValueRet);

        static bool SelectCoins(std::vector<COutPoint>& vAvailableCoins, const CAmount& nTargetValue, std::set<COutPoint>& setCoinsRet, CAmount& nValueRet);

        static bool AvailableCoins(const std::string& pubKey, std::vector<COutPoint>& vCoins);

        static bool SelectRewards(std::vector<CTxReward>& vAvailableRewards, const CAmount& nTargetValue, std::vector<CTxReward>& setRewardsRet, CAmount& nValueRet);

        static bool AvailableRewards(const std::string& pubKey, std::vector<CTxReward>& vRewards);

        static CAmount GetMinimumFee(unsigned int nTxBytes, unsigned int nConfirmTarget, const CTxMemPool& pool);

        static int GetDepthInMainChain(CCoins& coins);

        static int GetBlocksToMaturity(CCoins& coins);
};

#endif //TAUCOIN_TX_UTILS_H
