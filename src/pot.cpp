// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "arith_uint256.h"
#include "base58.h"
#include "pot.h"
#include "amount.h"
#include "chain.h"
#include "chainparams.h"
#include "checkpoints.h"
#include "clubman.h"
#include "coins.h"
#include "consensus/validation.h"
#include "core_io.h"
#include "main.h"
#include "pubkey.h"
#include "policy/policy.h"
#include "primitives/transaction.h"
#include "rpc/server.h"
#include "streams.h"
#include "sync.h"
#include "txmempool.h"
#include "util.h"
#include "utilstrencodings.h"
#include "hash.h"
#include "tool.h"
#include <stdint.h>
#include <univalue.h>
#include <boost/thread/thread.hpp> // boost::thread::interrupt
#include "wallet/wallet.h"
#include "script/script.h"
#include <secp256k1.h>
#include <secp256k1_recovery.h>

#define MAXRATIO 67
#define MINRATIO 53
#define GAMMA 0.64

const uint256 DiffAdjustNumerator = uint256S("0x010000000000000000");
const arith_uint256 Arith256DiffAdjustNumerator = UintToArith256(DiffAdjustNumerator);
const uint256 DiffAdjustNumeratorHalf = uint256S("0x0100000000");
const uint256 DiffAdjustNumeratorfor55 = uint256S("0x80000000000000");
const arith_uint256 Arith256DiffAdjustNumeratorHalf = UintToArith256(DiffAdjustNumeratorHalf);

const static bool fDebugPODS = true;

#if 0
UniValue getLatestBlockHash(){
    LOCK(cs_main);
    uint32_t nHeight = chainActive.Height();
    std::cout<<"current main chain height is "<<nHeight<<std::endl;
    CBlockIndex* pblockindex = chainActive[nHeight];
    std::cout <<"hex is as follows "<<std::hex <<pblockindex->GetBlockHash().GetHex() <<std::endl;
    return pblockindex->GetBlockHash().GetHex();
}
uint256 getPosHash( UniValue value){
    std::string str1 = value.get_str();
    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << str1;
    return ss.GetHash();
}

CPubKey GetPubKeyForPackage(){
    boost::shared_ptr<CReserveScript> coinbaseScript;
    GetMainSignals().ScriptForMining(coinbaseScript);

    // If the keypool is exhausted, no script is returned at all.  Catch this.
    if (!coinbaseScript || coinbaseScript->reserveScript.empty()){
        std::cout<<" error please check your wallet"<<std::endl;
    }
    CPubKey pubkey;
    coinbaseScript->GetReservedKey(pubkey);
    std::vector<unsigned char> ret = ToByteVector(pubkey);
    std::string pukstr = HexStr(ret);
    std::cout<<"publick key string is like " << pukstr<<std::endl;
    return pubkey;
}
uint64_t signatureCompactWithPubkey(const uint256 &phash, std::vector<unsigned char>& vchSig,CPubKey pubkey){
    vchSig.resize(65);

    int rec = -1;
    secp256k1_ecdsa_recoverable_signature sig;
    static secp256k1_context* secp256k1_context_sign = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    //std::vector<unsigned char>  pubkey1 = ToByteVector(pubkey);
    int ret = secp256k1_ecdsa_sign_recoverable(secp256k1_context_sign, &sig, phash.begin(), pubkey.begin()+1, secp256k1_nonce_function_rfc6979, NULL);
    assert(ret);
    secp256k1_ecdsa_recoverable_signature_serialize_compact(secp256k1_context_sign, (unsigned char*)&vchSig[1], &rec, &sig);
    assert(ret);
    assert(rec != -1);
    vchSig[0] = 27 + rec + (true ? 4 : 0);
    std::cout <<"sizeof vchSig is "<<vchSig.size() <<" vchSig[0] "<<std::dec<<(int)vchSig[0]<<" "<<(int)vchSig[1]<<std::endl;
    //EncodeBase64(&vchSig[0], 8);
    uint64_t hit=0;
    memcpy(&hit,&vchSig[0],8);
    return hit;
}

#endif

#if 1
std::string getLatestBlockGenerationSignature(){
    //LOCK(cs_main);
    uint32_t nHeight = chainActive.Height();
    //std::cout<<"current main chain height is "<<nHeight<<std::endl;
    CBlockIndex* pblockindex = chainActive[nHeight];
    std::cout <<"hex is as follows "<<std::hex <<pblockindex->GetBlockGenerationSignature()<<std::endl;
    return pblockindex->GetBlockGenerationSignature();
}
int64_t getPastTimeFromLastestBlock(){
   //LOCK(cs_main);
    uint32_t nHeight = chainActive.Height();
    CBlockIndex* pblockindex = chainActive[nHeight];
    int64_t timeNow = GetTime();
    int64_t pos_s = timeNow - pblockindex->GetBlockTime();
    return pos_s;
}

uint64_t getLatestBlockBaseTarget(){
    LOCK(cs_main);
    uint32_t nHeight = chainActive.Height();
    CBlockIndex* pblockindex = chainActive[nHeight];
    return pblockindex->GetBlockBaseTarget();
}

uint256 getPosHash(std::string generationSignature,std::string pubKey){
    uint256 ret = Hash(generationSignature.begin(),generationSignature.end(),pubKey.begin(),pubKey.end());
    return ret;
}

std::string raiseGenerationSignature(std::string pukstr){
    std::string pSignature = getLatestBlockGenerationSignature();
    CPubKey pubkey(pukstr.begin(), pukstr.end());
    std::string strPubKey;
    if (pubkey.IsCompressed() && pubkey.Decompress()) {
        strPubKey = HexStr(ToByteVector(pubkey));
    } else if (!pubkey.IsCompressed()) {
        strPubKey = pukstr;
    }
    uint256 ret = Hash(pSignature.begin(),pSignature.end(),strPubKey.begin(),strPubKey.end());
    return HexStr(ret);
}

bool verifyGenerationSignature(std::string pGS,std::string generationSignature,std::string pukstr){
    CPubKey pubkey(pukstr.begin(), pukstr.end());
    std::string strPubKey;
    if (pubkey.IsCompressed() && pubkey.Decompress()) {
        strPubKey = HexStr(ToByteVector(pubkey));
    } else if (!pubkey.IsCompressed()) {
        strPubKey = pukstr;
    }
    uint256 ret = Hash(pGS.begin(),pGS.end(),strPubKey.begin(),strPubKey.end());
    return generationSignature == HexStr(ret);
}

std::string GetPubKeyForPackage(){
    boost::shared_ptr<CReserveScript> coinbaseScript;
    GetMainSignals().ScriptForPackage(coinbaseScript);

    // If the keypool is exhausted, no script is returned at all.  Catch this.
    if (!coinbaseScript || coinbaseScript->reserveScript.empty()){
        std::cout<<" error please check your wallet"<<std::endl;
    }
    CPubKey pubkey;
    pubkey = coinbaseScript->Packagerpubkey;
    std::string pukstr;
    if(pubkey.Decompress()){
       std::vector<unsigned char> ret = ToByteVector(pubkey);
       pukstr = HexStr(ret);
       std::cout<<"public key string is like " << pukstr<<std::endl;
    }
    return pukstr;
}
uint64_t calculateHitOfPOS(const uint256 &phash){
    LogPrintf("Generation Signature Hash is %s\n",HexStr(phash));
    uint64_t hit=0;
    memcpy(&hit,phash.begin(),8);
    arith_uint256 temp = hit+1;
    double logarithm = log(temp.getdouble()) - 2 * log(Arith256DiffAdjustNumeratorHalf.getdouble());
    logarithm = fabs(logarithm);
    uint64_t ulogarithm =  logarithm * 1000;
    arith_uint256 arihit = UintToArith256(DiffAdjustNumeratorfor55) * arith_uint256(ulogarithm) / 1000;
    return ArithToUint256(arihit).GetUint64(0);
}
//watch out that CblockHeader is parent of Cblock
uint64_t getNextPosRequired(const CBlockIndex* pindexLast){
   uint64_t baseTargetLimt = 153722867; //genesis block base target
   const CBlockIndex* ancestor = NULL;
   if(pindexLast -> nHeight < 3){
       return baseTargetLimt;
   }else{
       ancestor = pindexLast->GetAncestor((pindexLast->nHeight) - 3);
   }
   uint64_t lastTime = pindexLast -> GetBlockTime();
   uint64_t lastBlockParentTime = ancestor->GetBlockTime();
   uint64_t Sa = (lastTime - lastBlockParentTime)/3 ;
   uint64_t newBaseTarget = 0;
   if (Sa > 60){
       uint64_t min = 0;
       if(Sa < MAXRATIO){
           min = Sa;
       }else{
           min = MAXRATIO;
       }
       newBaseTarget = (min * pindexLast->baseTarget)/60;
   }else{
       uint64_t max = 0;
       if(Sa > MINRATIO){
           max = Sa;
       }else{
           max = MINRATIO;
       }
       newBaseTarget = pindexLast->baseTarget - ((60 - max) * GAMMA * pindexLast->baseTarget)/60;
   }
   LogPrintf("NewBaseTarget is %s last time is %s lastBlockParentTime %s\n",newBaseTarget,lastTime,lastBlockParentTime);
   return newBaseTarget;
}

uint256 GetNextCumulativeDifficulty(const CBlockIndex* pindexLast, uint64_t baseTarget, const Consensus::Params& consensusParams)
{
    if (pindexLast == NULL) {
        // return genesis cumulative difficulty
        return consensusParams.genesisCumulativeDifficulty;
    }

    if (baseTarget == 0) {
        return pindexLast->cumulativeDifficulty;
    }

    assert(baseTarget);
    const arith_uint256 prevCumDiff = UintToArith256(pindexLast->cumulativeDifficulty);
    const arith_uint256 denominator(baseTarget);
    arith_uint256 temp(Arith256DiffAdjustNumerator);
    arith_uint256 ret(prevCumDiff);

    temp /= denominator;
    ret  += temp;

    return ArithToUint256(ret);
}

bool CheckProofOfTransaction(const std::string& prevGenerationSignature, const std::string& currPubKey,
        int nHeight, unsigned int nTime, uint64_t baseTarget, const Consensus::Params& consensusParams, PodsErr& checkErr)
{
    static ClubManager* pClubMgr = NULL;
    if (!pClubMgr)
        pClubMgr = ClubManager::GetInstance();

    checkErr = PODS_NO_ERR;

    if (prevGenerationSignature.empty() || currPubKey.empty() || nHeight < 0 || nTime == 0) {
        LogPrintf("CheckProofOfTransaction failed, incorrect args, signatrue:%s, pubkey:%s, height:%d, time:%d\n",
            prevGenerationSignature, currPubKey, nHeight, nTime);
        checkErr = PODS_ARGS_ERR;
        return false;
    }

    if (fDebugPODS) {
        LogPrintf("CheckProofOfTransaction, signature:%s, pubkey:%s\n", prevGenerationSignature, currPubKey);
        LogPrintf("CheckProofOfTransaction, height:%d, time:%d, baseTarget:%d\n", nHeight, nTime, baseTarget);
    }

    // Note: in block header public key is compressed, but get hit value with uncompressed
    // public key. So here firstly, we decompress publickey.
    CPubKey pubkey(currPubKey.begin(), currPubKey.end());
    std::string strPubKey;
    if (pubkey.IsCompressed() && pubkey.Decompress()) {
        strPubKey = HexStr(ToByteVector(pubkey));
    } else if (!pubkey.IsCompressed()) {
        strPubKey = currPubKey;
    }

    uint256 geneSignatureHash = getPosHash(prevGenerationSignature, strPubKey);
    uint64_t hit = calculateHitOfPOS(geneSignatureHash);
    std::string strAddr;

    if (!ConvertPubkeyToAddress(currPubKey, strAddr) || strAddr.empty()) {
        LogPrintf("CheckProofOfTransaction failed, get strAddr fail\n");
        checkErr = PODS_ADDR_ERR;
        return false;
    }
    if (fDebugPODS)
        LogPrintf("CheckProofOfTransaction, strAddr:%s\n", strAddr);

    // get harverst power with nHeight
    uint64_t harverstPower = pClubMgr->GetHarvestPowerByAddress(strAddr, nHeight);
    // If harverst power is one, return false directly.
    if (harverstPower <= ClubManager::DEFAULT_HARVEST_POWER) {
        LogPrintf("CheckProofOfTransaction failed, not allow forge\n");
        checkErr = PODS_BALANCE_ERR;
        return false;
    }

    arith_uint256 thresold(baseTarget);
    thresold *= arith_uint256((uint64_t)nTime);
    thresold *= arith_uint256((uint64_t)harverstPower);

    LogPrintf("CheckProofOfTransaction, harverst Power:%d\n", harverstPower);
    LogPrintf("CheckProofOfTransaction, hit     :%s\n", arith_uint256(hit).ToString());
    LogPrintf("CheckProofOfTransaction, thresold:%s\n", thresold.ToString());
    if (thresold.CompareTo(arith_uint256(hit)) > 0) {
        return true;
    }

    return false;
}

#endif
