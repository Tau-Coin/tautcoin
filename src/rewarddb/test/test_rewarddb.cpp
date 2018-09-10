// Copyright (c) 2017-2018 The Taucoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "test_rewarddb.h"

//#include "chainparams.h"
//#include "consensus/consensus.h"
//#include "consensus/validation.h"
#include "key.h"
#include "main.h"
//#include "miner.h"
//#include "pubkey.h"
//#include "random.h"
//#include "txdb.h"
//#include "txmempool.h"
//#include "ui_interface.h"
//#include "rpc/server.h"
//#include "rpc/register.h"

#include "test/testutil.h"

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

//extern bool fPrintToConsole;
extern void noui_connect();


using namespace std;
namespace TAURewardDB
{
BasicTestingSetup::BasicTestingSetup(const std::string& chainName)
{
        ECC_Start();
        SetupEnvironment();
        SetupNetworking();
        fPrintToDebugLog = false; // don't want to write to debug.log file
        fCheckBlockIndex = true;
        SelectParams(chainName);
        noui_connect();
}

BasicTestingSetup::~BasicTestingSetup()
{
        ECC_Stop();
}

TestingSetup::TestingSetup(const std::string& chainName) : BasicTestingSetup(chainName)
{
    const CChainParams& chainparams = Params();
    // Ideally we'd move all the RPC tests to the functional testing framework
    // instead of unit tests, but for now we need these here.
    //RegisterAllCoreRPCCommands(tableRPC);
    //ClearDatadirCache();
    pathTemp = GetTempPath() / strprintf("test_bitcoin_%lu_%i", (unsigned long)GetTime(), (int)(GetRand(100000)));
    boost::filesystem::create_directories(pathTemp);
    mapArgs["-datadir"] = pathTemp.string();
    pclubinfodb = new CClubInfoDB();
    pmemberinfodb = new CMemberInfoDB(pclubinfodb);
    //mempool.setSanityCheck(1.0);
    pblocktree = new CBlockTreeDB(1 << 20, true);
    pcoinsdbview = new CCoinsViewDB(1 << 23, true);
    //pcoinsTip = new CCoinsViewCache(pcoinsdbview);
    InitBlockIndex(chainparams);
    nScriptCheckThreads = 3;
    for (int i=0; i < nScriptCheckThreads-1; i++)
        threadGroup.create_thread(&ThreadScriptCheck);
    RegisterNodeSignals(GetNodeSignals());
}

TestingSetup::~TestingSetup()
{
        UnregisterNodeSignals(GetNodeSignals());
        threadGroup.interrupt_all();
        threadGroup.join_all();
        UnloadBlockIndex();
        delete pcoinsTip;
        delete pcoinsdbview;
        delete pblocktree;
        delete pmemberinfodb;
        pmemberinfodb = NULL;
        delete pclubinfodb;
        pclubinfodb = NULL;
        boost::filesystem::remove_all(pathTemp);
}

}

static string GetRandomAddress()
{
    CKey secret;
    secret.MakeNewKey(true);
    CPubKey pubkey = secret.GetPubKey();

    //std::cout<<"{PublicKey: \""<<HexStr(ToByteVector(pubkey))<<"\","<<std::endl;
    //std::cout<<"PrivateKey: \""<<CBitcoinSecret(secret).ToString()<<"\","<<std::endl;

    const CScript genesisOutputScript0 = CScript() << ParseHex(HexStr(ToByteVector(pubkey))) << OP_CHECKSIG;
    CBitcoinAddress addr;
    std::string address;
    addr.ScriptPub2Addr(genesisOutputScript0, address);
    //std::cout<<"Address: \""<<address<<"\"}"<<std::endl;

    return address;
}

BOOST_FIXTURE_TEST_SUITE(rewarddb_tests, TAURewardDB::TestingSetup)

static const unsigned int NUM_SIMULATION_ITERATIONS = 40000;
static const unsigned int NUM_SIMULATION_ADDRESSS = 1000000;

BOOST_AUTO_TEST_CASE(coins_cache_simulation_test)
{

}

BOOST_AUTO_TEST_SUITE_END()

//TestChain100Setup::TestChain100Setup() : TestingSetup(CBaseChainParams::REGTEST)
//{
//    // Generate a 100-block chain:
//    coinbaseKey.MakeNewKey(true);
//    CScript scriptPubKey = CScript() <<  ToByteVector(coinbaseKey.GetPubKey()) << OP_CHECKSIG;
//    for (int i = 0; i < COINBASE_MATURITY; i++)
//    {
//        std::vector<CMutableTransaction> noTxns;
//        CBlock b = CreateAndProcessBlock(noTxns, scriptPubKey);
//        coinbaseTxns.push_back(b.vtx[0]);
//    }
//}

////
//// Create a new block with just given transactions, coinbase paying to
//// scriptPubKey, and try to add it to the current chain.
////
//CBlock
//TestChain100Setup::CreateAndProcessBlock(const std::vector<CMutableTransaction>& txns, const CScript& scriptPubKey)
//{
//    const CChainParams& chainparams = Params();
//    std::string pubkeyString = "this is a test network";
//    CBlockTemplate *pblocktemplate = BlockAssembler(chainparams).CreateNewBlock(scriptPubKey,pubkeyString);
//    CBlock& block = pblocktemplate->block;

//    // Replace mempool-selected txns with just coinbase plus passed-in txns:
//    block.vtx.resize(1);
//    BOOST_FOREACH(const CMutableTransaction& tx, txns)
//        block.vtx.push_back(tx);
//    // IncrementExtraNonce creates a valid coinbase and merkleRoot
//    unsigned int extraNonce = 0;
//    IncrementExtraNonce(&block, chainActive.Tip(), extraNonce);

//    //while (!CheckProofOfWork(block.GetHash(), block.nBits, chainparams.GetConsensus())) ++block.nNonce;

//    CValidationState state;
//    ProcessNewBlock(state, chainparams, NULL, &block, true, NULL);

//    CBlock result = block;
//    delete pblocktemplate;
//    return result;
//}

//TestChain100Setup::~TestChain100Setup()
//{
//}


//CTxMemPoolEntry TestMemPoolEntryHelper::FromTx(CMutableTransaction &tx, CTxMemPool *pool) {
//    CTransaction txn(tx);
//    return FromTx(txn, pool);
//}

//CTxMemPoolEntry TestMemPoolEntryHelper::FromTx(CTransaction &txn, CTxMemPool *pool) {
//    bool hasNoDependencies = pool ? pool->HasNoInputsOf(txn) : hadNoDependencies;
//    // Hack to assume either its completely dependent on other mempool txs or not at all
//    CAmount inChainValue = hasNoDependencies ? txn.GetValueOut() : 0;

//    return CTxMemPoolEntry(txn, nFee, nTime, dPriority, nHeight,
//                           hasNoDependencies, inChainValue, spendsCoinbase, sigOpCost, lp);
//}

//void Shutdown(void* parg)
//{
//  exit(0);
//}

//void StartShutdown()
//{
//  exit(0);
//}

//bool ShutdownRequested()
//{
//  return false;
//}









//typedef unsigned int uint;
//using namespace std;
//namespace
//{
//class CCoinsViewTest : public CCoinsView
//{
//    uint256 hashBestBlock_;
//    std::map<uint256, CCoins> map_;

//public:
//    bool GetCoins(const uint256& txid, CCoins& coins) const
//    {
//        std::map<uint256, CCoins>::const_iterator it = map_.find(txid);
//        if (it == map_.end()) {
//            return false;
//        }
//        coins = it->second;
//        if (coins.IsPruned() && insecure_rand() % 2 == 0) {
//            // Randomly return false in case of an empty entry.
//            return false;
//        }
//        return true;
//    }

//    bool HaveCoins(const uint256& txid) const
//    {
//        CCoins coins;
//        return GetCoins(txid, coins);
//    }

//    uint256 GetBestBlock() const { return hashBestBlock_; }

//    bool BatchWrite(CCoinsMap& mapCoins, const uint256& hashBlock)
//    {
//        for (CCoinsMap::iterator it = mapCoins.begin(); it != mapCoins.end(); ) {
//            if (it->second.flags & CCoinsCacheEntry::DIRTY) {
//                // Same optimization used in CCoinsViewDB is to only write dirty entries.
//                map_[it->first] = it->second.coins;
//                if (it->second.coins.IsPruned() && insecure_rand() % 3 == 0) {
//                    // Randomly delete empty entries on write.
//                    map_.erase(it->first);
//                }
//            }
//            mapCoins.erase(it++);
//        }
//        if (!hashBlock.IsNull())
//            hashBestBlock_ = hashBlock;
//        return true;
//    }
//};

//class CCoinsViewCacheTest : public CCoinsViewCache
//{
//public:
//    CCoinsViewCacheTest(CCoinsView* base) : CCoinsViewCache(base) {}

//    void SelfTest() const
//    {
//        // Manually recompute the dynamic usage of the whole data, and compare it.
//        size_t ret = memusage::DynamicUsage(cacheCoins);
//        for (CCoinsMap::iterator it = cacheCoins.begin(); it != cacheCoins.end(); it++) {
//            ret += it->second.coins.DynamicMemoryUsage();
//        }
//        BOOST_CHECK_EQUAL(DynamicMemoryUsage(), ret);
//    }

//};

//}

//BOOST_FIXTURE_TEST_SUITE(coins_tests, BasicTestingSetup)

//static const unsigned int NUM_SIMULATION_ITERATIONS = 40000;
//static const unsigned int NUM_SIMULATION_ADDRESSS = 1000000;

//// This is a large randomized insert/remove simulation test on a variable-size
//// stack of caches on top of CCoinsViewTest.
////
//// It will randomly create/update/delete CCoins entries to a tip of caches, with
//// txids picked from a limited list of random 256-bit hashes. Occasionally, a
//// new tip is added to the stack of caches, or the tip is flushed and removed.
////
//// During the process, booleans are kept to make sure that the randomized
//// operation hits all branches.
//BOOST_AUTO_TEST_CASE(coins_cache_simulation_test)
//{
//    // Various coverage trackers.
//    bool removed_all_caches = false;
//    bool reached_4_caches = false;
//    bool added_an_entry = false;
//    bool removed_an_entry = false;
//    bool updated_an_entry = false;
//    bool found_an_entry = false;
//    bool missed_an_entry = false;

//    // A simple map to track what we expect the cache stack to represent.
//    std::map<uint256, CCoins> result;

//    // The cache stack.
//    CCoinsViewTest base; // A CCoinsViewTest at the bottom.
//    std::vector<CCoinsViewCacheTest*> stack; // A stack of CCoinsViewCaches on top.
//    stack.push_back(new CCoinsViewCacheTest(&base)); // Start with one cache.

//    // Use a limited set of random transaction ids, so we do test overwriting entries.
//    std::vector<uint256> txids;
//    txids.resize(NUM_SIMULATION_ITERATIONS / 8);
//    for (unsigned int i = 0; i < txids.size(); i++) {
//        txids[i] = GetRandHash();
//    }

//    for (unsigned int i = 0; i < NUM_SIMULATION_ITERATIONS; i++) {
//        // Do a random modification.
//        {
//            uint256 txid = txids[insecure_rand() % txids.size()]; // txid we're going to modify in this iteration.
//            CCoins& coins = result[txid];
//            CCoinsModifier entry = stack.back()->ModifyCoins(txid);
//            BOOST_CHECK(coins == *entry);
//            if (insecure_rand() % 5 == 0 || coins.IsPruned()) {
//                if (coins.IsPruned()) {
//                    added_an_entry = true;
//                } else {
//                    updated_an_entry = true;
//                }
//                coins.nVersion = insecure_rand();
//                coins.vout.resize(1);
//                coins.vout[0].nValue = insecure_rand();
//                *entry = coins;
//            } else {
//                coins.Clear();
//                entry->Clear();
//                removed_an_entry = true;
//            }
//        }

//        // Once every 1000 iterations and at the end, verify the full cache.
//        if (insecure_rand() % 1000 == 1 || i == NUM_SIMULATION_ITERATIONS - 1) {
//            for (std::map<uint256, CCoins>::iterator it = result.begin(); it != result.end(); it++) {
//                const CCoins* coins = stack.back()->AccessCoins(it->first);
//                if (coins) {
//                    BOOST_CHECK(*coins == it->second);
//                    found_an_entry = true;
//                } else {
//                    BOOST_CHECK(it->second.IsPruned());
//                    missed_an_entry = true;
//                }
//            }
//            BOOST_FOREACH(const CCoinsViewCacheTest *test, stack) {
//                test->SelfTest();
//            }
//        }

//        if (insecure_rand() % 100 == 0) {
//            // Every 100 iterations, flush an intermediate cache
//            if (stack.size() > 1 && insecure_rand() % 2 == 0) {
//                unsigned int flushIndex = insecure_rand() % (stack.size() - 1);
//                stack[flushIndex]->Flush();
//            }
//        }
//        if (insecure_rand() % 100 == 0) {
//            // Every 100 iterations, change the cache stack.
//            if (stack.size() > 0 && insecure_rand() % 2 == 0) {
//                //Remove the top cache
//                stack.back()->Flush();
//                delete stack.back();
//                stack.pop_back();
//            }
//            if (stack.size() == 0 || (stack.size() < 4 && insecure_rand() % 2)) {
//                //Add a new cache
//                CCoinsView* tip = &base;
//                if (stack.size() > 0) {
//                    tip = stack.back();
//                } else {
//                    removed_all_caches = true;
//                }
//                stack.push_back(new CCoinsViewCacheTest(tip));
//                if (stack.size() == 4) {
//                    reached_4_caches = true;
//                }
//            }
//        }
//    }

//    // Clean up the stack.
//    while (stack.size() > 0) {
//        delete stack.back();
//        stack.pop_back();
//    }

//    // Verify coverage.
//    BOOST_CHECK(removed_all_caches);
//    BOOST_CHECK(reached_4_caches);
//    BOOST_CHECK(added_an_entry);
//    BOOST_CHECK(removed_an_entry);
//    BOOST_CHECK(updated_an_entry);
//    BOOST_CHECK(found_an_entry);
//    BOOST_CHECK(missed_an_entry);
//}
