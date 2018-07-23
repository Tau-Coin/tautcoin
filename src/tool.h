// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2018- The imorpheus Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ISNCOIN_TOOL_H
#define ISNCOIN_TOOL_H

#include "amount.h"
#include "base58.h"
#include "script/script.h"

bool ConvertPubkeyToAddress(const std::string& pubKey, std::string& addrStr);

bool ConvertPubKeyIntoBitAdress(const CScript& script, CBitcoinAddress& addr);

#endif //ISNCOIN_TOOL_H
