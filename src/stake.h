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

bool ConvertPubkeyToAddress(const std::string& pubKey, std::string& addrStr);

bool IsAllowForge(const std::string pubKey, int height);

CAmount GetEffectiveBalance(const std::string address, int nHeight);

bool isForgeScript(const CScript& script, CBitcoinAddress& addr, int& memCount);

bool ConvertPubKeyIntoBitAdress(const CScript& script, CBitcoinAddress& addr);

#endif //IMCOIN_STAKE_H