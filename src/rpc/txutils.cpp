// Copyright (c) 2018- The Taucoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txutils.h"

#include "base58.h"
#include "chain.h"
#include "coins.h"
#include "consensus/validation.h"
#include "core_io.h"
#include "init.h"
#include "keystore.h"
#include "main.h"
#include "merkleblock.h"
#include "net.h"
#include "policy/policy.h"
#include "primitives/transaction.h"
#include "rpc/server.h"
#include "random.h"
#include "rewardman.h"
#include "script/script.h"
#include "script/script_error.h"
#include "script/sign.h"
#include "script/standard.h"
#include "tool.h"
#include "txmempool.h"
#include "uint256.h"
#include "utilstrencodings.h"
#include "utilmoneystr.h"
#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#endif

#include <stdint.h>

#include <boost/assign/list_of.hpp>

#include <univalue.h>
using namespace std;

struct CompareValueOnly
{
    bool operator()(const std::pair<CAmount, COutPoint>& t1,
                    const std::pair<CAmount, COutPoint>& t2) const
    {
        return t1.first < t2.first;
    }
};

struct CompareRewardOnly
{
    bool operator()(const CTxReward& t1,
                    const CTxReward& t2) const
    {
        return t1.rewardBalance < t2.rewardBalance;
    }
};

static void ApproximateBestSubset(std::vector<std::pair<CAmount, COutPoint> >vValue, const CAmount& nTotalLower, const CAmount& nTargetValue,
                                  std::vector<char>& vfBest, CAmount& nBest, int iterations = 1000)
{
    std::vector<char> vfIncluded;

    vfBest.assign(vValue.size(), true);
    nBest = nTotalLower;

    seed_insecure_rand();

    for (int nRep = 0; nRep < iterations && nBest != nTargetValue; nRep++)
    {
        vfIncluded.assign(vValue.size(), false);
        CAmount nTotal = 0;
        bool fReachedTarget = false;
        for (int nPass = 0; nPass < 2 && !fReachedTarget; nPass++)
        {
            for (unsigned int i = 0; i < vValue.size(); i++)
            {
                //The solver here uses a randomized algorithm,
                //the randomness serves no real security purpose but is just
                //needed to prevent degenerate behavior and it is important
                //that the rng is fast. We do not use a constant random sequence,
                //because there may be some privacy improvement by making
                //the selection random.
                if (nPass == 0 ? insecure_rand()&1 : !vfIncluded[i])
                {
                    nTotal += vValue[i].first;
                    vfIncluded[i] = true;
                    if (nTotal >= nTargetValue)
                    {
                        fReachedTarget = true;
                        if (nTotal < nBest)
                        {
                            nBest = nTotal;
                            vfBest = vfIncluded;
                        }
                        nTotal -= vValue[i].first;
                        vfIncluded[i] = false;
                    }
                }
            }
        }
    }
}

int CTransactionUtils::GetDepthInMainChain(CCoins& coins)
{
    LOCK(cs_main);
    return chainActive.Height() - coins.nHeight + 1;
}

int CTransactionUtils::GetBlocksToMaturity(CCoins& coins)
{
    if (!coins.IsCoinBase())
        return 0;
    return max(0, (COINBASE_MATURITY + 1) - GetDepthInMainChain(coins));
}

bool CTransactionUtils::SelectCoinsMinConf(const CAmount& nTargetValue, int nConf, std::vector<COutPoint>& vCoins,
        std::set<COutPoint>& setCoinsRet, CAmount& nValueRet)
{
    LogPrint("selectcoins", "%s entry\n", __func__);

    setCoinsRet.clear();
    nValueRet = 0;

    int tipHeight = 0;
    {
        LOCK(cs_main);
        tipHeight = chainActive.Height();
    }

    // List of values less than target
    std::pair<CAmount, COutPoint> coinLowestLarger;
    coinLowestLarger.first = std::numeric_limits<CAmount>::max();

    std::vector<std::pair<CAmount, COutPoint> > vValue;
    CAmount nTotalLower = 0;

    random_shuffle(vCoins.begin(), vCoins.end(), GetRandInt);

    BOOST_FOREACH(const COutPoint& output, vCoins)
    {
        CCoins coins;
        if (!pcoinsTip->GetCoins(output.hash, coins))
        {
            LogPrintf("%s get coins fail:%s\n", __func__, output.hash.ToString());
            return false;
        }

        int nDepth = tipHeight - coins.nHeight + 1;
        if ( nDepth < nConf)
            continue;

        int n = output.n;
        CAmount v = coins.vout[output.n].nValue;

        std::pair<CAmount, COutPoint> coin = std::make_pair(v, COutPoint(output.hash, n));

        if (v == nTargetValue)
        {
            setCoinsRet.insert(output);
            nValueRet += v;
            return true;
        }
        else if (v < nTargetValue + MIN_CHANGE)
        {
            vValue.push_back(coin);
            nTotalLower += v;
        }
        else if (v < coinLowestLarger.first)
        {
            coinLowestLarger = coin;
        }
    }

    LogPrint("selectcoins", "total lower:%d, target:%d\n", nTotalLower, nTargetValue);

    if (nTotalLower == nTargetValue)
    {
        for (unsigned int i = 0; i < vValue.size(); ++i)
        {
            setCoinsRet.insert(vValue[i].second);
            nValueRet += vValue[i].first;
        }
        return true;
    }

    if (nTotalLower < nTargetValue)
    {
        if (coinLowestLarger.second.IsNull())
        {
            for (unsigned int i = 0; i < vValue.size(); ++i)
            {
                setCoinsRet.insert(vValue[i].second);
            }
            nValueRet = nTotalLower;
            return false;
        }
        setCoinsRet.insert(coinLowestLarger.second);
        nValueRet += coinLowestLarger.first;
        return true;
    }

    // Solve subset sum by stochastic approximation
    std::sort(vValue.begin(), vValue.end(), CompareValueOnly());
    std::reverse(vValue.begin(), vValue.end());
    std::vector<char> vfBest;
    CAmount nBest;

    ApproximateBestSubset(vValue, nTotalLower, nTargetValue, vfBest, nBest);
    if (nBest != nTargetValue && nTotalLower >= nTargetValue + MIN_CHANGE)
        ApproximateBestSubset(vValue, nTotalLower, nTargetValue + MIN_CHANGE, vfBest, nBest);

    // If we have a bigger coin and (either the stochastic approximation didn't find a good solution,
    //                                   or the next bigger coin is closer), return the bigger coin
    if (((nBest != nTargetValue && nBest < nTargetValue + MIN_CHANGE) || coinLowestLarger.first <= nBest)
        && !coinLowestLarger.second.IsNull())
    {
        setCoinsRet.insert(coinLowestLarger.second);
        nValueRet += coinLowestLarger.first;
    }
    else {
        for (unsigned int i = 0; i < vValue.size(); i++)
            if (vfBest[i])
            {
                setCoinsRet.insert(vValue[i].second);
                nValueRet += vValue[i].first;
            }

        LogPrint("selectcoins", "SelectCoins() best subset: ");
        for (unsigned int i = 0; i < vValue.size(); i++)
            if (vfBest[i])
                LogPrint("selectcoins", "%s ", FormatMoney(vValue[i].first));
        LogPrint("selectcoins", "total %s\n", FormatMoney(nBest));
    }

    return true;
}

bool CTransactionUtils::SelectCoins(std::vector<COutPoint>& vAvailableCoins, const CAmount& nTargetValue, std::set<COutPoint>& setCoinsRet, CAmount& nValueRet)
{
    bool res = SelectCoinsMinConf(nTargetValue, 6, vAvailableCoins, setCoinsRet, nValueRet) ||
        SelectCoinsMinConf(nTargetValue, 1, vAvailableCoins, setCoinsRet, nValueRet);

    return res;
}

bool CTransactionUtils::AvailableCoins(const std::string& pubKey, std::vector<COutPoint>& vCoins)
{
    vCoins.clear();
    std::string addrStr;
    if (ConvertPubkeyToAddress(pubKey, addrStr))
    {
        LogPrint("selectcoins", "%s addr:%s\n", __func__, addrStr);
    }

    CScript script;
    CBitcoinAddress address(addrStr);
    if (address.IsValid())
    {
        script = GetScriptForDestination(address.Get());
    }
    else if (IsHex(pubKey))
    {
        std::vector<unsigned char> data(ParseHex(pubKey));
        script = CScript(data.begin(), data.end());
    }
    else
    {
        LogPrintf("Invalid Taucoin address or script: %s\n", pubKey);
        return false;
    }

    CCoinsByScript coinsByScript;
    pcoinsByScript->GetCoinsByScript(script, coinsByScript);

    BOOST_FOREACH(const COutPoint &outpoint, coinsByScript.setCoins)
    {
        CCoins coins;
        if (!pcoinsTip->GetCoins(outpoint.hash, coins))
        {
            LogPrintf("get coins failed: %s\n", outpoint.hash.ToString());
            continue;
        }

        if (coins.IsCoinBase() && GetBlocksToMaturity(coins) > 0)
            continue;

        if (outpoint.n < coins.vout.size() && !coins.vout[outpoint.n].IsNull()
                && coins.vout[outpoint.n].nValue > 0 && !coins.vout[outpoint.n].scriptPubKey.IsUnspendable())
        {
            vCoins.push_back(COutPoint(outpoint.hash, outpoint.n));
        }
    }

    return true;
}

bool CTransactionUtils::SelectRewards(std::vector<CTxReward>& vAvailableRewards, const CAmount& nTargetValue, std::vector<CTxReward>& setRewardsRet, CAmount& nValueRet)
{
    // Here vAvailableCoins.size() == 1
    if (vAvailableRewards.size() < 1)
        return false;

    setRewardsRet.clear();
    nValueRet = 0;

    // Here, suppose vAvailableRewards.size() == 1
    const CTxReward& rw = vAvailableRewards[0];
    if (rw.rewardBalance < nTargetValue)
        return false;

    nValueRet = rw.rewardBalance;
    setRewardsRet.push_back(rw);

    return true;
}

bool CTransactionUtils::AvailableRewards(const std::string& pubKey, std::vector<CTxReward>& vRewards)
{
    vRewards.clear();
    std::string addrStr;
    if (ConvertPubkeyToAddress(pubKey, addrStr))
    {
        LogPrint("selectcoins", "%s addr:%s\n", __func__, addrStr);
    }

    CAmount rewards = RewardManager::GetInstance()->GetRewardsByPubkey(pubKey);

    vRewards.push_back(CTxReward(pubKey, rewards, (uint32_t)GetTime()));

    return true;
}

CAmount CTransactionUtils::GetMinimumFee(unsigned int nTxBytes, unsigned int nConfirmTarget, const CTxMemPool& pool)
{
    CAmount nFeeNeeded = 0;
    // use -txconfirmtarget to estimate...
    int estimateFoundTarget = nConfirmTarget;
    nFeeNeeded = pool.estimateSmartFee(nConfirmTarget, &estimateFoundTarget).GetFee(nTxBytes);

    // prevent user from paying a fee below minRelayTxFees
    nFeeNeeded = std::max(nFeeNeeded, ::minRelayTxFee.GetFee(nTxBytes));
    // But always obey the maximum
    if (nFeeNeeded > DEFAULT_TRANSACTION_MAXFEE)
        nFeeNeeded = DEFAULT_TRANSACTION_MAXFEE;

    return nFeeNeeded;
}

bool CTransactionUtils::CreateTransaction(std::map<std::string, CAmount>& receipts, const bool bSubtractFeeFromReceipts, const std::string& pubKey,
        const std::string& prvKey, CFeeRate& userFee, CMutableTransaction& tx, CAmount& nFeeRet, std::string& strFailReason)
{
    LogPrint("rpc", "%s entry\n", __func__);

    // Firstly, construct recipinets
    std::vector<CRecipient> vecSend;
    CAmount nTotal = 0;
    for (std::map<std::string, CAmount> ::iterator i = receipts.begin();
            i != receipts.end(); i++)
    {
        LogPrint("rpc", "receipt address: %s\n", i->first);
        CBitcoinAddress addr(i->first);
        if (!addr.IsValid())
        {
            strFailReason = _("Error, CreateTransaction, invalid address");
            return false;
        }
        CScript scriptPubKey = GetScriptForDestination(CBitcoinAddress(i->first).Get());
        CRecipient recipient = {scriptPubKey, i->second, bSubtractFeeFromReceipts};
        vecSend.push_back(recipient);
        nTotal += i->second;
    }

    // Generate key store
    CBasicKeyStore keystore;
    CBitcoinSecret vchSecret;
    if (!vchSecret.SetString(prvKey))
    {
        strFailReason = _("incorrect private key");
        return false;
    }
    CKey key = vchSecret.GetKey();
    if (!key.IsValid())
    {
        strFailReason = _("incorrect private key");
        return false;
    }
    keystore.AddKey(key);

    {
        LOCK(cs_main);
        tx.nLockTime = chainActive.Height();
    }

    if (GetRandInt(10) == 0)
        tx.nLockTime = std::max(0, (int)tx.nLockTime - GetRandInt(100));

    {
        LOCK(cs_main);
        assert(tx.nLockTime <= (unsigned int)chainActive.Height());
    }
    assert(tx.nLockTime < LOCKTIME_THRESHOLD);

    std::vector<COutPoint> coins;
    std::vector<CTxReward> vAvailableRewards;
    if (!AvailableCoins(pubKey, coins))
    {
        AvailableRewards(pubKey, vAvailableRewards);
        if (vAvailableRewards.size() == 0)
        {
            strFailReason = _("fail to get UTXOs and rewards, invalid address");
            return false;
        }
    }

    if (coins.size() == 0)
    {
        strFailReason = _("no UTXOs");
        return false;
    }

    LogPrint("selectcoins", "available coins:[\n");
    BOOST_FOREACH(const COutPoint& c, coins)
    {
        LogPrint("selectcoins", "\t%s,\t%d\n", c.hash.ToString(), c.n);
    }
    LogPrint("selectcoins", "]\n");

    CAmount nValue = nTotal;
    unsigned int nSubtractFeeFromAmount = 0;
    if (bSubtractFeeFromReceipts) {
        nSubtractFeeFromAmount = receipts.size();
    }
    nFeeRet = 0;

    // Start with no fee and loop until there is enough fee
    while (true)
    {
        tx.vin.clear();
        tx.vreward.clear();
        tx.vout.clear();

        bool fFirst = true;

        CAmount nValueToSelect = nValue;
        if (nSubtractFeeFromAmount == 0)
            nValueToSelect += nFeeRet;

        // vouts to the payees
        BOOST_FOREACH (const CRecipient& recipient, vecSend)
        {
            CTxOut txout(recipient.nAmount, recipient.scriptPubKey);

            if (recipient.fSubtractFeeFromAmount)
            {
                txout.nValue -= nFeeRet / nSubtractFeeFromAmount; // Subtract fee equally from each selected recipient

                if (fFirst) // first receiver pays the remainder not divisible by output count
                {
                    fFirst = false;
                    txout.nValue -= nFeeRet % nSubtractFeeFromAmount;
                }
            }

            if (txout.IsDust(::minRelayTxFee))
            {
                if (recipient.fSubtractFeeFromAmount && nFeeRet > 0)
                {
                    if (txout.nValue < 0)
                        strFailReason = _("The transaction amount is too small to pay the fee");
                    else
                        strFailReason = _("The transaction amount is too small to send after the fee has been deducted");
                }
                else
                    strFailReason = _("Transaction amount too small");
                return false;
            }
            tx.vout.push_back(txout);
        }

        // Choose coins to use
        std::set<COutPoint> setCoins;
        std::vector<CTxReward> setRewards;
        CAmount nValueIn = 0;
        CAmount nRewardIn = 0;
        LogPrint("selectcoins", "selected value:%d\n", nValueToSelect);
        if (!SelectCoins(coins, nValueToSelect, setCoins, nValueIn))
        {
            if (vAvailableRewards.size() == 0)
            {
                AvailableRewards(pubKey, vAvailableRewards);
            }

            CAmount nRewardTargetValue = nValueToSelect - nValueIn;
            LogPrint("selectcoins", "selected reward target value:%d  [\n", nRewardTargetValue);
            if (!SelectRewards(vAvailableRewards, nRewardTargetValue, setRewards, nRewardIn))
            {
                strFailReason = _("Insufficient funds");
                return false;
            }
        }

        LogPrint("selectcoins", "selected coins value:%d  [\n", nValueIn);
        BOOST_FOREACH(const COutPoint& c, setCoins)
        {
            LogPrint("selectcoins", "\t%s,\t%d\n", c.hash.ToString(), c.n);
        }
        LogPrint("selectcoins", "]\n");

        LogPrint("selectcoins", "selected rewards value:%d  [\n", nRewardIn);
        BOOST_FOREACH(const CTxReward& rw, setRewards)
        {
            LogPrint("selectcoins", "\t%s,\t%d\n", rw.senderPubkey, rw.rewardBalance);
        }
        LogPrint("selectcoins", "]\n");

        nValueIn += nRewardIn;
        const CAmount nChange = nValueIn - nValueToSelect;
        if (nChange > 0)
        {
            // Fill a vout to ourself
            CScript scriptChange;
            std::string addrStr;
            if (!ConvertPubkeyToAddress(pubKey, addrStr))
            {
                strFailReason = _("Convert change pubkey error");
                return false;
            }

            CBitcoinAddress address(addrStr);
            if (address.IsValid())
            {
                scriptChange = GetScriptForDestination(address.Get());
            }
            else if (IsHex(pubKey))
            {
                std::vector<unsigned char> data(ParseHex(pubKey));
                scriptChange = CScript(data.begin(), data.end());
            }
            else
            {
                strFailReason = _("public key is incorrect");
                return false;
            }

            CTxOut newTxOut(nChange, scriptChange);

            // We do not move dust-change to fees, because the sender would end up paying more than requested.
            // This would be against the purpose of the all-inclusive feature.
            // So instead we raise the change and deduct from the recipient.
            if (nSubtractFeeFromAmount > 0 && newTxOut.IsDust(::minRelayTxFee))
            {
                CAmount nDust = newTxOut.GetDustThreshold(::minRelayTxFee) - newTxOut.nValue;
                newTxOut.nValue += nDust; // raise change until no more dust
                for (unsigned int i = 0; i < vecSend.size(); i++) // subtract from first recipient
                {
                    if (vecSend[i].fSubtractFeeFromAmount)
                    {
                        tx.vout[i].nValue -= nDust;
                        if (tx.vout[i].IsDust(::minRelayTxFee))
                        {
                            strFailReason = _("The transaction amount is too small to send after the fee has been deducted");
                            return false;
                        }
                        break;
                    }
                }
            }

            // Never create dust outputs; if we would, just
            // add the dust to the fee.
            if (newTxOut.IsDust(::minRelayTxFee))
            {
                nFeeRet += nChange;
            }
            else
            {
                // Insert change txn at random position:
                int nChangePosInOut = GetRandInt(tx.vout.size() + 1);

                std::vector<CTxOut>::iterator position = tx.vout.begin() + nChangePosInOut;
                tx.vout.insert(position, newTxOut);
            }
        }

        // Fill vin
        //
        // Note how the sequence number is set to max()-1 so that the
        // nLockTime set above actually works.
        BOOST_FOREACH(const COutPoint& coin, setCoins)
            tx.vin.push_back(CTxIn(coin, CScript(),
                                      std::numeric_limits<unsigned int>::max()-1));
        // Fill vreward
        BOOST_FOREACH(const CTxReward& reward, setRewards)
            tx.vreward.push_back(reward);

        // Sign
        int nIn = 0;
        CTransaction txNewConst(tx);
        BOOST_FOREACH(const COutPoint& outpoint, setCoins)
        {
            CCoins coin;
            if (!pcoinsTip->GetCoins(outpoint.hash, coin))
            {
                strFailReason = _("fail to get UTXOs");
                return false;
            }

            bool signSuccess;
            const CScript& scriptPubKey = coin.vout[outpoint.n].scriptPubKey;
            SignatureData sigdata;
            signSuccess = ProduceSignature(TransactionSignatureCreator(&keystore, &txNewConst, nIn, coin.vout[outpoint.n].nValue, SIGHASH_ALL),
                    scriptPubKey, sigdata);

            if (!signSuccess)
            {
                strFailReason = _("Signing transaction failed");
                return false;
            } else {
                UpdateTransaction(tx, nIn, sigdata);
            }

            nIn++;
        }

        int nReward = 0;
        if (setRewards.size() > 0)// Sign rewards
        {
            BOOST_FOREACH(const CTxReward& reward, setRewards)
            {
                bool signSuccess;
                bool bCheckReward = true;
                const CScript scriptPubKey = CScript() << ParseHex(reward.senderPubkey) << OP_CHECKREWARDSIG;
                LogPrint("rpc", "sign rewards pubkey:%s \n", reward.senderPubkey);
                SignatureData sigdata;

                signSuccess = ProduceSignatureForRewards(TransactionSignatureCreator(&keystore, &txNewConst, nReward, reward.rewardBalance, SIGHASH_ALL, bCheckReward), scriptPubKey, sigdata);

                if (!signSuccess)
                {
                    strFailReason = _("Signing transaction failed with rewards");
                    return false;
                } else {
                    tx.vreward[nReward].scriptSig = sigdata.scriptSig;
                    //UpdateTransaction(txNew, nIn, sigdata);
                }

                nReward++;
            }
        }

        unsigned int nBytes = GetVirtualTransactionSize(tx);

        //LogPrintf("%s %d bytes\n", __func__, nBytes);

        // Limit size
        if (GetTransactionWeight(tx) >= MAX_STANDARD_TX_WEIGHT)
        {
            strFailReason = _("Transaction too large");
            return false;
        }

        CAmount nFeeNeeded;
        {
            LOCK(cs_main);
            nFeeNeeded = GetMinimumFee(nBytes, nTxConfirmTarget, mempool);
        }

        CAmount nUserFee = userFee.GetFee(nBytes);

        //LogPrintf("%s user fee:%d, need fee:%d\n", __func__, nUserFee, nFeeNeeded);

        if (nUserFee > nFeeNeeded) {
            nFeeNeeded = nUserFee;
        }

        if (nFeeNeeded < ::minRelayTxFee.GetFee(nBytes))
        {
            nFeeNeeded = ::minRelayTxFee.GetFee(nBytes);
        }

        if (nFeeRet >= nFeeNeeded)
        {
            //LogPrintf("%s return fee:%d, need fee:%d\n", __func__, nFeeRet, nFeeNeeded);
            break; // Done, enough fee included.
        }

        // Include more fee and try again.
        nFeeRet = nFeeNeeded;
        continue;
    }

    LogPrint("rpc", "create transaction successfully\n");
    return true;
}

bool CTransactionUtils::SendTransaction(const CMutableTransaction& mutx, const CAmount fee, uint256& hashTx, std::string& failReason)
{
    CTransaction tx(mutx);
    hashTx = tx.GetHash();

    const CCoins* existingCoins;
    bool fHaveMempool = false;
    {
        LOCK(cs_main);
        existingCoins = pcoinsTip->AccessCoins(hashTx);
        fHaveMempool = mempool.exists(hashTx);
    }
    bool fHaveChain = existingCoins && existingCoins->nHeight < 1000000000;

    if (!fHaveMempool && !fHaveChain) {
        // push to local node and sync with wallets
        CValidationState state;
        bool fMissingInputs;
        {
            LOCK(cs_main);
            if (!AcceptToMemoryPool(mempool, state, tx, false, &fMissingInputs, false, fee))
            {
                if (state.IsInvalid()) {
                    failReason = state.GetRejectReason();
                    LogPrintf("%s fail: %d %s\n", __func__, state.GetRejectCode(), failReason);
                    return false;
                } else {
                    if (fMissingInputs) {
                        failReason = _("Missing inputs");
                        return false;
                    }
                    failReason = state.GetRejectReason();
                    return false;
                }
            }
        }
    } else if (fHaveChain) {
        failReason = _("transaction already in block chain");
        return false;
    }

    RelayTransaction(tx);

    return true;
}
