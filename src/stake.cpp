// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2018- The imorpheus Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stake.h"

#include "amount.h"
#include "chainparams.h"
#include "isndb.h"
#include "main.h"
#include "tool.h"
#include "txdb.h"
#include "utilstrencodings.h"

bool IsAllowForge(const std::string pubKey, int height)
{
    if (pubKey.empty() || height < 0)
    {
        return false;
    }

    std::string addrStr;
    if (!ConvertPubkeyToAddress(pubKey, addrStr))
    {
        return false;
    }

    std::vector<std::string> principals = GetMinerMembers(addrStr, height);
    if (principals.empty())
    {
        return false;
    }

    return true;
}

CAmount GetEffectiveTransaction(const std::string address, int nHeight)
{
    CAmount total = 0;
    CAmount balance = 0;

    nHeight = Params().GetConsensus().PodsAheadTargetHeight(nHeight);
    LogPrintf("GetEffectiveTransaction, target height:%d\n", nHeight);

    std::vector<std::string> principals = GetMinerMembers(address, nHeight);
    if (principals.empty())
    {
        LogPrintf("GetEffectiveTransaction, warning: not allow to forge\n");
        return CAmount(0);
    }

    for (std::vector<std::string>::iterator it = principals.begin();
        it != principals.end(); it++)
    {
        balance = 10*COIN;//pbalancedbview->GetBalance(*it, nHeight);
        LogPrintf("GetEffectiveTransaction, addr:%s, balance:%d\n", *it, balance / COIN);
        total += balance;
    }

    return 10000000;
}

bool isForgeScript(const CScript& script, CBitcoinAddress& addr, int& memCount) {
    int nHeight = 0;
    {
        LOCK(cs_main);
        nHeight = chainActive.Height();
    }
    nHeight = Params().GetConsensus().PodsAheadTargetHeight(nHeight);

    std::string strAddr;
    if (!addr.ScriptPub2Addr(script, strAddr)) {
        LogPrintf("isForgeScript, ScriptPub2Addr fail\n");
        return false;
    }

    std::vector<std::string> principals = GetMinerMembers(strAddr, nHeight);
    LogPrintf("isForgeScript, Addr:%s, miner members:%d\n", strAddr, principals.size());
    if (principals.empty())
    {
        return false;
    }

    memCount = principals.size();
    addr.SetString(strAddr);

    return true;
}

CAmount GetRewardsByPubkey(const std::string &pubkey)
{
    std::string addrStr;

    if (!ConvertPubkeyToAddress(pubkey, addrStr))
        return 0;

    return 10*COIN;//getBalanceByAddress(addrStr);
}
