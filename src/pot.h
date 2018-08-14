// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2018- The imorpheus Core developers at pos
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TAUCOIN_POS_H
#define TAUCOIN_POS_H
#include "chainparams.h"
#include "consensus/params.h"
#include <stdint.h>
#include <univalue.h>
#include "sync.h"
#include <iostream>
#include "pubkey.h"
class CBlockHeader;
class CBlockIndex;
class uint256;
//verify your account's right to package block
//algorithm is h  < Tb * S * Be
//Tb = Tb * t (target before * 2016 average time of forging block)
//S is the time when you get your right
//Be is your UTXO before
//h is signature using your pubkey to sig last block

extern const arith_uint256 Arith256DiffAdjustNumerator;

typedef enum {
    POT_NO_ERR,
    POT_ARGS_ERR,
    POT_ADDR_ERR,
    POT_BALANCE_ERR,
    POT_ERR_NUMBER
} PotErr;

UniValue getLatestBlockHash();
//CPubKey GetPubKeyForPackage();
uint256 getPotHash( UniValue value);
uint64_t signatureCompactWithPubkey(const uint256 &phash, std::vector<unsigned char>& vchSig,CPubKey pubkey);

std::string getLatestBlockGenerationSignature();
uint256 getPotHash(std::string generationSignature,std::string pubKey);
std::string GetPubKeyForPackage();
uint64_t calculateHitOfPOT(const uint256 &phash);
std::string raiseGenerationSignature(std::string pukstr);
bool verifyGenerationSignature(std::string pGS,std::string generationSignature,std::string pukstr);
int64_t getPastTimeFromLastestBlock();
uint64_t getLatestBlockBaseTarget();
uint64_t getNextPotRequired(const CBlockIndex* pindexLast);

uint256 GetNextCumulativeDifficulty(const CBlockIndex* pindexLast, uint64_t baseTarget, const Consensus::Params& consensusParams);

bool CheckProofOfTransaction(const std::string& prevGenerationSignature, const std::string& currPubKey,
        int nHeight, int64_t nTime, uint64_t baseTarget, uint64_t harverstPower, const Consensus::Params& consensusParams, PotErr& checkErr);

#endif // TAUCOIN_POS_H
