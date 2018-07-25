// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2018- The imorpheus Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef IMCOIN_STAKE_H
#define IMCOIN_STAKE_H

#include "amount.h"
#include "base58.h"
#include "script/script.h"

bool IsAllowForge(const std::string pubKey, int height);

CAmount GetEffectiveTransaction(const std::string address, int nHeight);

bool isForgeScript(const CScript& script, CBitcoinAddress& addr, int& memCount);

CAmount GetRewardsByPubkey(const std::string &pubkey);

#endif //IMCOIN_STAKE_H
