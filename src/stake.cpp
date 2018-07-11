// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2018- The imorpheus Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stake.h"

#include "amount.h"
#include "base58.h"
#include "chainparams.h"
#include "main.h"
#include "script/script.h"
#include "txdb.h"
#include "utilstrencodings.h"

extern CBalanceViewDB *pbalancedbview;

bool ConvertPubkeyToAddress(const std::string& pubKey, std::string& addrStr)
{
    if (pubKey.empty())
        return false;

    const CScript script = CScript() << ParseHex(pubKey) << OP_CHECKSIG;
    CBitcoinAddress addr;

    bool ret = addr.ScriptPub2Addr(script, addrStr);
    return ret;
}

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

CAmount GetEffectiveBalance(const std::string address, int nHeight)
{
    CAmount total = 0;
    CAmount balance = 0;

    nHeight = Params().GetConsensus().PodsAheadTargetHeight(nHeight);
    LogPrintf("GetEffectiveBalance, target height:%d\n", nHeight);

    std::vector<std::string> principals = GetMinerMembers(address, nHeight);
    if (principals.empty())
    {
        LogPrintf("GetEffectiveBalance, warning: not allow to forge\n");
        return CAmount(0);
    }

    for (std::vector<std::string>::iterator it = principals.begin();
        it != principals.end(); it++)
    {
        balance = pbalancedbview->GetBalance(*it, nHeight);
        LogPrintf("GetEffectiveBalance, addr:%s, balance:%d\n", *it, balance);
        total += balance;
    }

    return total / COIN;
}
