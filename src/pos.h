// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2018- The imorpheus Core developers at pos
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef IMCOIN_POS_H
#define IMCOIN_POS_H
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
UniValue getLatestBlockHash();
//CPubKey GetPubKeyForPackage();
uint256 getPosHash( UniValue value);
uint64_t signatureCompactWithPubkey(const uint256 &phash, std::vector<unsigned char>& vchSig,CPubKey pubkey);

std::string getLatestBlockGenerationSignature();
uint256 getPosHash(std::string generationSignature,std::string pubKey);
std::string GetPubKeyForPackage();
uint64_t calculateHitOfPOS(const uint256 &phash);
std::string raiseGenerationSignature(std::string pukstr);
bool verifyGenerationSignature(std::string generationSignature,std::string pukstr);
int64_t getPastTimeFromLastestBlock();
uint64_t getLatestBlockBaseTarget();
uint64_t getNextPosRequired(const CBlockIndex* pindexLast);

uint256 getNextCumulativeDifficulty(const CBlockIndex* pindexLast, const CChainParams& chainparams);

bool VerifyProofOfStake(const std::string& prevGenerationSignature, const std::string& currPubKey,
        int nHeight, const Consensus::Params& consensusParams);

#endif // IMCOIN_POS_H
