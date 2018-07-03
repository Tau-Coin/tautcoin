// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pos.h"
#include "amount.h"
#include "chain.h"
#include "chainparams.h"
#include "checkpoints.h"
#include "coins.h"
#include "consensus/validation.h"
#include "core_io.h"
#include "main.h"
#include "policy/policy.h"
#include "primitives/transaction.h"
#include "rpc/server.h"
#include "streams.h"
#include "sync.h"
#include "txmempool.h"
#include "util.h"
#include "utilstrencodings.h"
#include "hash.h"
#include <stdint.h>
#include <univalue.h>
#include <boost/thread/thread.hpp> // boost::thread::interrupt
#include "wallet/wallet.h"
#include "script/script.h"
#include <secp256k1.h>
#include <secp256k1_recovery.h>

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
    LOCK(cs_main);
    uint32_t nHeight = chainActive.Height();
    std::cout<<"current main chain height is "<<nHeight<<std::endl;
    CBlockIndex* pblockindex = chainActive[nHeight];
    std::cout <<"hex is as follows "<<std::hex <<pblockindex->GetBlockGenerationSignature()<<std::endl;
    return pblockindex->GetBlockGenerationSignature();
}
int64_t getPastTimeFromLastestBlock(){
    LOCK(cs_main);
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
    uint256 ret1 = Hash(generationSignature.begin(),generationSignature.end(),pubKey.begin(),pubKey.end());
    std::cout<<"ret1 is "<<ret1.ToString()<<" ret2 "<<HexStr(ret1)<<std::endl;
    return ret1;
}

std::string raiseGenerationSignature(std::string pukstr){
    uint256 ret = Hash(pukstr.begin(),pukstr.end());
    return HexStr(ret);
}

bool verifyGenerationSignature(std::string generationSignature,std::string pukstr){
    uint256 ret = Hash(pukstr.begin(),pukstr.end());
    //std::cout<<" genesis verify "<<HexStr(ret)<<std::endl;
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
       std::cout<<"publick key string is like " << pukstr<<std::endl;
    }
    return pukstr;
}
uint64_t calculateHitOfPOS(const uint256 &phash){
    std::cout <<" generation signature hash is "<<HexStr(phash)<<std::endl;
    //EncodeBase64(phash.begin(), phash.size())<<std::endl;
    uint64_t hit=0;
    memcpy(&hit,phash.begin(),8);
    return hit;
}
//注意这里CblockHeader是Cblock的父类，用多态的形式实现 const CBlockHeader *pblock
uint64_t getNextPosRequired(const CBlockIndex* pindexLast){
   uint64_t baseTargetLimt = 153722867; //genesis block base target
   if(pindexLast == NULL){
       return baseTargetLimt;
   }
   //if block height 1
   if(pindexLast->pprev == NULL){
      return baseTargetLimt;
   }
   uint64_t lastTime = pindexLast -> GetBlockTime();
   uint64_t lastBlockParentTime = pindexLast->pprev -> GetBlockTime();
   uint64_t newBaseTarget = (lastTime - lastBlockParentTime)*pindexLast->baseTarget;
   std::cout<<"NewBaseTarget is "<<newBaseTarget<<" last time is "<<lastTime<<" lastBlockParentTime "<<lastBlockParentTime<<std::endl;
   return newBaseTarget;
}

#endif
