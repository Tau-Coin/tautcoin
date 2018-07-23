// Copyright (c) 2018- The imorpheus Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "tool.h"

#include "amount.h"
#include "chainparams.h"
#include "main.h"
#include "txdb.h"
#include "utilstrencodings.h"

bool ConvertPubkeyToAddress(const std::string& pubKey, std::string& addrStr)
{
    if (pubKey.empty())
        return false;

    const CScript script = CScript() << ParseHex(pubKey) << OP_CHECKSIG;
    CBitcoinAddress addr;

    bool ret = addr.ScriptPub2Addr(script, addrStr);
    return ret;
}

bool ConvertPubKeyIntoBitAdress(const CScript& script, CBitcoinAddress& addr) {
    std::string strAddr;
    if (!addr.ScriptPub2Addr(script, strAddr)) {
        LogPrintf("ConvertPubKeyIntoBitAdress, ScriptPub2Addr fail\n");
        return false;
    }

    addr.SetString(strAddr);
    return true;
}
