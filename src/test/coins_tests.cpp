// Copyright (c) 2014-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "coins.h"
#include "random.h"
#include "script/standard.h"
#include "uint256.h"
#include "utilstrencodings.h"
#include "test/test_bitcoin.h"
#include "main.h"
#include "consensus/validation.h"
#include "rewardman.h"

#include <vector>
#include <map>

#include <boost/test/unit_test.hpp>

typedef unsigned int uint;

namespace
{
class CCoinsViewTest : public CCoinsView
{
    uint256 hashBestBlock_;
    std::map<uint256, CCoins> map_;

public:
    bool GetCoins(const uint256& txid, CCoins& coins) const
    {
        std::map<uint256, CCoins>::const_iterator it = map_.find(txid);
        if (it == map_.end()) {
            return false;
        }
        coins = it->second;
        if (coins.IsPruned() && insecure_rand() % 2 == 0) {
            // Randomly return false in case of an empty entry.
            return false;
        }
        return true;
    }

    bool HaveCoins(const uint256& txid) const
    {
        CCoins coins;
        return GetCoins(txid, coins);
    }

    uint256 GetBestBlock() const { return hashBestBlock_; }

    bool BatchWrite(CCoinsMap& mapCoins, const uint256& hashBlock)
    {
        for (CCoinsMap::iterator it = mapCoins.begin(); it != mapCoins.end(); ) {
            if (it->second.flags & CCoinsCacheEntry::DIRTY) {
                // Same optimization used in CCoinsViewDB is to only write dirty entries.
                map_[it->first] = it->second.coins;
                if (it->second.coins.IsPruned() && insecure_rand() % 3 == 0) {
                    // Randomly delete empty entries on write.
                    map_.erase(it->first);
                }
            }
            mapCoins.erase(it++);
        }
        if (!hashBlock.IsNull())
            hashBestBlock_ = hashBlock;
        return true;
    }
};

class CCoinsViewCacheTest : public CCoinsViewCache
{
public:
    CCoinsViewCacheTest(CCoinsView* base) : CCoinsViewCache(base) {}

    void SelfTest() const
    {
        // Manually recompute the dynamic usage of the whole data, and compare it.
        size_t ret = memusage::DynamicUsage(cacheCoins);
        for (CCoinsMap::iterator it = cacheCoins.begin(); it != cacheCoins.end(); it++) {
            ret += it->second.coins.DynamicMemoryUsage();
        }
        BOOST_CHECK_EQUAL(DynamicMemoryUsage(), ret);
    }

};

}

BOOST_FIXTURE_TEST_SUITE(coins_tests, BasicTestingSetup)

static const unsigned int NUM_SIMULATION_ITERATIONS = 40000;
static const unsigned int NUM_SIMULATION_ADDRESSS = 1000000;

// This is a large randomized insert/remove simulation test on a variable-size
// stack of caches on top of CCoinsViewTest.
//
// It will randomly create/update/delete CCoins entries to a tip of caches, with
// txids picked from a limited list of random 256-bit hashes. Occasionally, a
// new tip is added to the stack of caches, or the tip is flushed and removed.
//
// During the process, booleans are kept to make sure that the randomized
// operation hits all branches.
BOOST_AUTO_TEST_CASE(coins_cache_simulation_test)
{
    // Various coverage trackers.
    bool removed_all_caches = false;
    bool reached_4_caches = false;
    bool added_an_entry = false;
    bool removed_an_entry = false;
    bool updated_an_entry = false;
    bool found_an_entry = false;
    bool missed_an_entry = false;

    // A simple map to track what we expect the cache stack to represent.
    std::map<uint256, CCoins> result;

    // The cache stack.
    CCoinsViewTest base; // A CCoinsViewTest at the bottom.
    std::vector<CCoinsViewCacheTest*> stack; // A stack of CCoinsViewCaches on top.
    stack.push_back(new CCoinsViewCacheTest(&base)); // Start with one cache.

    // Use a limited set of random transaction ids, so we do test overwriting entries.
    std::vector<uint256> txids;
    txids.resize(NUM_SIMULATION_ITERATIONS / 8);
    for (unsigned int i = 0; i < txids.size(); i++) {
        txids[i] = GetRandHash();
    }

    for (unsigned int i = 0; i < NUM_SIMULATION_ITERATIONS; i++) {
        // Do a random modification.
        {
            uint256 txid = txids[insecure_rand() % txids.size()]; // txid we're going to modify in this iteration.
            CCoins& coins = result[txid];
            CCoinsModifier entry = stack.back()->ModifyCoins(txid);
            BOOST_CHECK(coins == *entry);
            if (insecure_rand() % 5 == 0 || coins.IsPruned()) {
                if (coins.IsPruned()) {
                    added_an_entry = true;
                } else {
                    updated_an_entry = true;
                }
                coins.nVersion = insecure_rand();
                coins.vout.resize(1);
                coins.vout[0].nValue = insecure_rand();
                *entry = coins;
            } else {
                coins.Clear();
                entry->Clear();
                removed_an_entry = true;
            }
        }

        // Once every 1000 iterations and at the end, verify the full cache.
        if (insecure_rand() % 1000 == 1 || i == NUM_SIMULATION_ITERATIONS - 1) {
            for (std::map<uint256, CCoins>::iterator it = result.begin(); it != result.end(); it++) {
                const CCoins* coins = stack.back()->AccessCoins(it->first);
                if (coins) {
                    BOOST_CHECK(*coins == it->second);
                    found_an_entry = true;
                } else {
                    BOOST_CHECK(it->second.IsPruned());
                    missed_an_entry = true;
                }
            }
            BOOST_FOREACH(const CCoinsViewCacheTest *test, stack) {
                test->SelfTest();
            }
        }

        if (insecure_rand() % 100 == 0) {
            // Every 100 iterations, flush an intermediate cache
            if (stack.size() > 1 && insecure_rand() % 2 == 0) {
                unsigned int flushIndex = insecure_rand() % (stack.size() - 1);
                stack[flushIndex]->Flush();
            }
        }
        if (insecure_rand() % 100 == 0) {
            // Every 100 iterations, change the cache stack.
            if (stack.size() > 0 && insecure_rand() % 2 == 0) {
                //Remove the top cache
                stack.back()->Flush();
                delete stack.back();
                stack.pop_back();
            }
            if (stack.size() == 0 || (stack.size() < 4 && insecure_rand() % 2)) {
                //Add a new cache
                CCoinsView* tip = &base;
                if (stack.size() > 0) {
                    tip = stack.back();
                } else {
                    removed_all_caches = true;
                }
                stack.push_back(new CCoinsViewCacheTest(tip));
                if (stack.size() == 4) {
                    reached_4_caches = true;
                }
            }
        }
    }

    // Clean up the stack.
    while (stack.size() > 0) {
        delete stack.back();
        stack.pop_back();
    }

    // Verify coverage.
    BOOST_CHECK(removed_all_caches);
    BOOST_CHECK(reached_4_caches);
    BOOST_CHECK(added_an_entry);
    BOOST_CHECK(removed_an_entry);
    BOOST_CHECK(updated_an_entry);
    BOOST_CHECK(found_an_entry);
    BOOST_CHECK(missed_an_entry);
}

static CBlock BuildBlockTestCase1(const CAmount* leaderVout, uint coinbaseVoutCnt, uint num, string pubkey)
{
    CBlock block;
    CMutableTransaction tx;

    const CScript outputScript = CScript() << ParseHex(pubkey) << OP_CHECKSIG;
    if (num < coinbaseVoutCnt) // The tx is coinbase
    {
        tx.vin.resize(1);
        tx.vin[0].scriptSig.resize(10);
        tx.vin[0].prevout.SetNull();

        tx.vout.resize(1);
        tx.vout[0].nValue = leaderVout[num];
        tx.vout[0].scriptPubKey = outputScript;

        block.vtx.resize(1);
        block.vtx[0] = tx;
        block.nVersion = 42;
        block.hashPrevBlock = GetRandHash();
    }

    block.hashMerkleRoot = GetRandHash();
    return block;
}

static CBlock BuildBlockTestCase2(CAmount totalRewards, double voutRatio, string pubkey)
{
    CBlock block;
    CMutableTransaction tx;

    const CScript outputScript = CScript() << ParseHex(pubkey) << OP_CHECKSIG;

    tx.vin.resize(1);
    tx.vin[0].scriptSig.resize(10);
    tx.vin[0].prevout.SetNull();

    tx.vout.resize(1);
    tx.vout[0].nValue = totalRewards * voutRatio;
    tx.vout[0].scriptPubKey = outputScript;

    block.vtx.resize(1);
    block.vtx[0] = tx;
    block.nVersion = 42;
    block.hashPrevBlock = GetRandHash();

    block.hashMerkleRoot = GetRandHash();
    return block;
}

static CBlock BuildBlockTestCase3(const CAmount* memberRBalance, const string* memberPubkey)
{
    CBlock block;
    CMutableTransaction tx;

    block.vtx.resize(1);
    block.nVersion = 42;
    block.hashPrevBlock = GetRandHash();

    tx.vin.resize(10);
    for (size_t i = 0; i < tx.vin.size(); i++) {
        tx.vin[i].prevout.hash = GetRandHash();
        tx.vin[i].prevout.n = 0;
    }
    tx.vreward.resize(3);
    for (size_t i = 0; i < tx.vreward.size(); i++) {
        tx.vreward[i].senderPubkey = memberPubkey[i];
        tx.vreward[i].scriptSig.resize(10);
        tx.vreward[i].rewardBalance = memberRBalance[i];
    }
    block.vtx[0] = tx;

    block.hashMerkleRoot = GetRandHash();
    return block;
}

static long AddNewLeader(ISNDB* pdb, string addr)
{
    long clubId;
    std::vector<std::string> values;

    values.push_back(addr);
    values.push_back("1");
    clubId = pdb->ISNSqlInsert(tableClub, values);

    values.clear();
    values.push_back(addr);
    values.push_back(std::to_string(clubId));
    values.push_back("0");
    values.push_back("1");
    values.push_back("0");
    pdb->ISNSqlInsert(tableMember, values);

    return clubId;
}

static void AddNewMemberToClub(ISNDB* pdb, string leaderAddr, string memberAddr)
{
    std::vector<std::string> field, tableMemberValues;
    field.clear();
    field.push_back(memFieldID);
    field.push_back(memFieldClub);
    mysqlpp::StoreQueryResult data = pdb->ISNSqlSelectAA(tableMember, field, memFieldAddress, leaderAddr);
    tableMemberValues.clear();
    tableMemberValues.push_back(memberAddr);
    tableMemberValues.push_back(data[0]["club_id"].c_str());
    tableMemberValues.push_back(data[0]["address_id"].c_str());
    tableMemberValues.push_back("0");
    tableMemberValues.push_back("0");
    pdb->ISNSqlInsert(tableMember, tableMemberValues);
}

static void TCAddOneByAddress(ISNDB* pdb, string address)
{
    std::vector<std::string> field;
    field.clear();
    field.push_back(memFieldCount);
    pdb->ISNSqlAddOne(tableMember, field, memFieldAddress, address);
}

static void TTCAddOneByAddress(ISNDB* pdb, string leaderAddress)
{
    std::vector<std::string> field;
    field.clear();
    field.push_back(memFieldClub);
    mysqlpp::StoreQueryResult data = pdb->ISNSqlSelectAA(tableMember, field, memFieldAddress, leaderAddress);
    field.clear();
    field.push_back(clubFieldCount);
    pdb->ISNSqlAddOne(tableClub, field, clubFieldID, data[0]["club_id"].c_str());
}

static string ScriptToPubKey(const CScript& script)
{
    CTxDestination dest;
    ExtractDestination(script, dest);
    string addr = CBitcoinAddress(dest).ToString();//TQNJigbPKJJJMapz49h4uMPqvW7wWgJMnC
    return addr;
}

//BOOST_AUTO_TEST_CASE(updaterewards_simulation_test)
//{
//    // Init
//    mapArgs["-mysqldbname"] = "imreward";
//    mapArgs["-mysqlserver"] = "localhost";
//    mapArgs["-mysqlusername"] = "root";
//    mapArgs["-mysqlpassword"] = "Mp3895";
//    mapArgs["-leaderreward"] = "0.5";
//    const uint memberCnt = 3;
//    string testPubkey = "039bb9e9c7f2602721e8f53fdcc1d6583afae76463156db2bda3673f0f11543dde";
//    string testmemberPubkey[memberCnt] = {
//        "0321b6dc3bfc5a5904afb3b431c5636c855f831f8ef7fc7e9b8c2c3a891d066388",
//        "03cb7f6c0381f98b96d260298a124abfeb9eec98aaa98cfa6d4e95032b1b5b773a",
//        "03705869ebbbe22bb84bcd96a3097884e6f818eadaee1520f9a9838aeca070e5a7"
//    };
//    const CScript outputScript = CScript() << ParseHex(testPubkey) << OP_CHECKSIG;
//    const CScript memberScript[memberCnt] = {
//        CScript() << ParseHex(testmemberPubkey[0]) << OP_CHECKSIG,
//        CScript() << ParseHex(testmemberPubkey[1]) << OP_CHECKSIG,
//        CScript() << ParseHex(testmemberPubkey[2]) << OP_CHECKSIG,
//    };
//    string testAddr = ScriptToPubKey(outputScript);//TQNJigbPKJJJMapz49h4uMPqvW7wWgJMnC
//    string memberAddr[memberCnt];
//    // Here start ISNDB service ASAP
//    ISNDB::StartISNDBService();
//    // Construct DB
//    ISNDB* pdb = ISNDB::GetInstance();
//    AddNewLeader(pdb, testAddr);

//    // Create inputs
//    const string ratiostr = mapArgs["-leaderreward"];
//    const double leaderRatio = atof(ratiostr.c_str());
//    const int height = 100;
//    const uint voutCnt = 5;
//    const uint TotalrewardsCnt = 3;
//    const uint tcLeaderCaseCnt = 3;
//    const uint tcCaseCnt = 3;
//    const uint testTime = voutCnt*TotalrewardsCnt + tcLeaderCaseCnt;
//    const uint clubTtc[tcCaseCnt] = {5, 8, 8};
//    const CAmount leaderVout[voutCnt] = {-2, 0, 10, 10000, 20000};
//    const CAmount TotalRewards[TotalrewardsCnt] = {-5, 0, 10000};
//    const arith_uint256 tcCase[tcCaseCnt][memberCnt] = {{0, 0, 0}, {2, 0, 1}, {3, 3, 3}};
//    const arith_uint256 tcLeaderCase = 5;
//    const CAmount memberRBalance[memberCnt] = {3333, 0, 1666};
//    RewardManager* rewardMan = RewardManager::GetInstance();
//    vector<CBlock> blocks;
//    for(uint i = 0; i < memberCnt; i++)
//    {
//        //TKPDyXLTh7mwUp1wLqzJCZZvFRKhWZcao8, TGntmw2qpvZc7aTpUr9dYYGduaRmvD6pWX, TGPtvy6P5RcUnvGSs7cbhYw2RC3VqnwvWA
//        memberAddr[i] = ScriptToPubKey(memberScript[i]);
//    }
//    for (uint i = 0; i < testTime; i++)
//    {
//        if (i < voutCnt*TotalrewardsCnt)
//        {
//            CBlock block(BuildBlockTestCase1(leaderVout, voutCnt, i, testPubkey));
//            blocks.push_back(block);
//        }
//        else if(i >= voutCnt*TotalrewardsCnt && i < voutCnt*TotalrewardsCnt + tcLeaderCaseCnt)
//        {
//            CBlock block(BuildBlockTestCase2(TotalRewards[2], leaderRatio, testPubkey));
//            blocks.push_back(block);
//        }
//        else if(i >= voutCnt*TotalrewardsCnt + tcLeaderCaseCnt)
//        {
//            CBlock block(BuildBlockTestCase3(memberRBalance, testmemberPubkey));
//            blocks.push_back(block);
//        }
//    }

//    // Create outputs
//    CAmount rewardbalance_new[testTime];
//    CAmount rbalanceMember_new[tcCaseCnt][memberCnt];
//    CAmount rbalanceMember_new_case3[memberCnt];
//    for (uint i = 0; i < testTime; i++)
//    {
//        if (i < voutCnt*TotalrewardsCnt)
//        {
//            CAmount rewardOld = rewardMan->GetRewardsByAddress(testAddr);
//            assert(rewardOld == 0);
//            rewardbalance_new[i] = TotalRewards[i/voutCnt] - leaderVout[i%voutCnt];
//        }
//        else if(i >= voutCnt*TotalrewardsCnt && i < voutCnt*TotalrewardsCnt + tcLeaderCaseCnt)
//        {
//            assert(rewardMan->GetRewardsByAddress(testAddr) == 0);
//            CAmount TotalMemberRewards = TotalRewards[2] * (1 - leaderRatio);

//            assert(rewardMan->GetRewardsByAddress(memberAddr[0]) == 0);
//            assert(rewardMan->GetRewardsByAddress(memberAddr[1]) == 0);
//            assert(rewardMan->GetRewardsByAddress(memberAddr[2]) == 0);
//            double memberttc = clubTtc[i-voutCnt*TotalrewardsCnt]-tcLeaderCase.getdouble();
//            rbalanceMember_new[i-voutCnt*TotalrewardsCnt][0] = memberttc == 0 ? 0 :
//                    (tcCase[i-voutCnt*TotalrewardsCnt][0].getdouble() / memberttc * TotalMemberRewards);
//            rbalanceMember_new[i-voutCnt*TotalrewardsCnt][1] = memberttc == 0 ? 0 :
//                    (tcCase[i-voutCnt*TotalrewardsCnt][1].getdouble() / memberttc * TotalMemberRewards);
//            rbalanceMember_new[i-voutCnt*TotalrewardsCnt][2] = memberttc == 0 ? 0 :
//                    (tcCase[i-voutCnt*TotalrewardsCnt][2].getdouble() / memberttc * TotalMemberRewards);

//            rewardbalance_new[i] = TotalMemberRewards - rbalanceMember_new[i-voutCnt*TotalrewardsCnt][0] -
//                     - rbalanceMember_new[i-voutCnt*TotalrewardsCnt][1] - rbalanceMember_new[i-voutCnt*TotalrewardsCnt][2];
//        }
//        else if(i >= voutCnt*TotalrewardsCnt + tcLeaderCaseCnt)
//        {
//            rbalanceMember_new_case3[0] = 0;
//            rbalanceMember_new_case3[1] = 0;
//            rbalanceMember_new_case3[2] = 0;
//        }
//    }

//    // Execute tests
//    bool ret[testTime];
//    CAmount rewardbalance_ret[testTime];
//    CAmount rbalanceMember_ret[tcCaseCnt][memberCnt];
//    CAmount rbalanceMember_ret_case3[memberCnt];
//    for (uint i = 0; i < testTime; i++)
//    {
//        CBlock block;
//        if (i < voutCnt*TotalrewardsCnt)
//        {
//            block = blocks[i % voutCnt];
//            for (uint j = 0; j < block.vtx.size(); j++)
//            {
//                const CTransaction tx = block.vtx[j];
//                ret[i] = UpdateRewards(tx, TotalRewards[i / voutCnt], height);
//                rewardbalance_ret[i] = rewardMan->GetRewardsByAddress(testAddr);
//                CAmount rewardOld = rewardMan->GetRewardsByAddress(testAddr);
//                assert(rewardMan->UpdateRewardsByAddress(testAddr, 0, rewardOld));
//                assert(rewardMan->GetRewardsByAddress(testAddr) == 0);
//            }
//        }
//        else if(i >= voutCnt*TotalrewardsCnt && i < voutCnt*TotalrewardsCnt + tcLeaderCaseCnt)
//        {
//            if (i == voutCnt*TotalrewardsCnt)
//            {
//                for(uint i = 0; i < memberCnt; i++)
//                    AddNewMemberToClub(pdb, testAddr, memberAddr[i]);

//                for(uint i = 0; i < clubTtc[0]-1; i++)
//                {
//                    TTCAddOneByAddress(pdb, testAddr);
//                    TCAddOneByAddress(pdb, testAddr);
//                }
//            }
//            else if(i == voutCnt*TotalrewardsCnt + 1)
//            {
//                TTCAddOneByAddress(pdb, testAddr);
//                TTCAddOneByAddress(pdb, testAddr);
//                TTCAddOneByAddress(pdb, testAddr);
//                TCAddOneByAddress(pdb, memberAddr[0]);
//                TCAddOneByAddress(pdb, memberAddr[0]);
//                TCAddOneByAddress(pdb, memberAddr[2]);
//            }
//            else if(i == voutCnt*TotalrewardsCnt + 2)
//            {
//                TCAddOneByAddress(pdb, memberAddr[0]);
//                TCAddOneByAddress(pdb, memberAddr[1]);
//                TCAddOneByAddress(pdb, memberAddr[1]);
//                TCAddOneByAddress(pdb, memberAddr[1]);
//                TCAddOneByAddress(pdb, memberAddr[2]);
//                TCAddOneByAddress(pdb, memberAddr[2]);
//            }

//            block = blocks[i];
//            for (uint j = 0; j < block.vtx.size(); j++)
//            {
//                const CTransaction tx = block.vtx[j];
//                ret[i] = UpdateRewards(tx, TotalRewards[2], height);
//                rewardbalance_ret[i] = rewardMan->GetRewardsByAddress(testAddr);
//                rbalanceMember_ret[i-voutCnt*TotalrewardsCnt][0] = rewardMan->GetRewardsByAddress(memberAddr[0]);
//                rbalanceMember_ret[i-voutCnt*TotalrewardsCnt][1] = rewardMan->GetRewardsByAddress(memberAddr[1]);
//                rbalanceMember_ret[i-voutCnt*TotalrewardsCnt][2] = rewardMan->GetRewardsByAddress(memberAddr[2]);
//                CAmount rewardOld = rewardMan->GetRewardsByAddress(testAddr);
//                assert(rewardMan->UpdateRewardsByAddress(testAddr, 0, rewardOld));
//                assert(rewardMan->GetRewardsByAddress(testAddr) == 0);
//                rewardOld = rewardMan->GetRewardsByAddress(memberAddr[0]);
//                assert(rewardMan->UpdateRewardsByAddress(memberAddr[0], 0, rewardOld));
//                assert(rewardMan->GetRewardsByAddress(memberAddr[0]) == 0);
//                rewardOld = rewardMan->GetRewardsByAddress(memberAddr[1]);
//                assert(rewardMan->UpdateRewardsByAddress(memberAddr[1], 0, rewardOld));
//                assert(rewardMan->GetRewardsByAddress(memberAddr[1]) == 0);
//                rewardOld = rewardMan->GetRewardsByAddress(memberAddr[2]);
//                assert(rewardMan->UpdateRewardsByAddress(memberAddr[2], 0, rewardOld));
//                assert(rewardMan->GetRewardsByAddress(memberAddr[2]) == 0);
//            }
//        }
//        else if(i >= voutCnt*TotalrewardsCnt + tcLeaderCaseCnt)
//        {
//            rewardMan->UpdateRewardsByAddress(testAddr, memberRBalance[0], -1);
//            assert(rewardMan->GetRewardsByAddress(testAddr) == memberRBalance[0]);
//            rewardMan->UpdateRewardsByAddress(testAddr, memberRBalance[1], -1);
//            assert(rewardMan->GetRewardsByAddress(testAddr) == memberRBalance[1]);
//            rewardMan->UpdateRewardsByAddress(testAddr, memberRBalance[2], -1);
//            assert(rewardMan->GetRewardsByAddress(testAddr) == memberRBalance[2]);
//            block = blocks[i];
//            for (uint j = 0; j < block.vtx.size(); j++)
//            {
//                const CTransaction tx = block.vtx[j];
//                ret[i] = UpdateRewards(tx, TotalRewards[2], height);
//                rewardbalance_ret[i] = rewardMan->GetRewardsByAddress(testAddr);
//            }
//            rbalanceMember_ret_case3[0] = rewardMan->GetRewardsByAddress(memberAddr[0]);
//            rbalanceMember_ret_case3[1] = rewardMan->GetRewardsByAddress(memberAddr[1]);
//            rbalanceMember_ret_case3[2] = rewardMan->GetRewardsByAddress(memberAddr[2]);
//        }
//    }

//    // Verify
//    for (uint i = 0; i < testTime; i++)
//    {
//        if (i < 5 || (i > 6 && i < 10) || i == 14)
//            BOOST_CHECK_MESSAGE(ret[i] == false, "case: " << i);
//        else if(i < voutCnt*TotalrewardsCnt)
//        {
//            BOOST_CHECK_MESSAGE(ret[i] == true, "case: " << i);
//            BOOST_CHECK_MESSAGE(rewardbalance_new[i] == rewardbalance_ret[i], "case: " << i);
//        }
//        else if(i == voutCnt*TotalrewardsCnt)
//        {
//            BOOST_CHECK_MESSAGE(ret[i] == true, "case: " << i);
//            BOOST_CHECK_MESSAGE(rewardbalance_new[i] == rewardbalance_ret[i], "case: " << i);
//            BOOST_CHECK_MESSAGE(rbalanceMember_new[0][0] == rbalanceMember_ret[0][0], "case: " << i);
//            BOOST_CHECK_MESSAGE(rbalanceMember_new[0][1] == rbalanceMember_ret[0][1], "case: " << i);
//            BOOST_CHECK_MESSAGE(rbalanceMember_new[0][2] == rbalanceMember_ret[0][2], "case: " << i);
//        }
//        else if(i == voutCnt*TotalrewardsCnt + 1)
//        {
//            BOOST_CHECK_MESSAGE(ret[i] == true, "case: " << i);
//            BOOST_CHECK_MESSAGE(rewardbalance_new[i] == rewardbalance_ret[i], "case: " << i);
//            BOOST_CHECK_MESSAGE(rbalanceMember_new[1][0] == rbalanceMember_ret[1][0], "case: " << i);
//            BOOST_CHECK_MESSAGE(rbalanceMember_new[1][1] == rbalanceMember_ret[1][1], "case: " << i);
//            BOOST_CHECK_MESSAGE(rbalanceMember_new[1][2] == rbalanceMember_ret[1][2], "case: " << i);
//        }
//        else if(i == voutCnt*TotalrewardsCnt + 2)
//            BOOST_CHECK_MESSAGE(ret[i] == false, "case: " << i);
//        else if(i >= voutCnt*TotalrewardsCnt + tcLeaderCaseCnt)
//        {
//            BOOST_CHECK_MESSAGE(ret[i] == true, "case: " << i);
//            BOOST_CHECK_MESSAGE(rbalanceMember_new_case3[0] == rbalanceMember_ret_case3[0], "case: " << i);
//            BOOST_CHECK_MESSAGE(rbalanceMember_new_case3[1] == rbalanceMember_ret_case3[1], "case: " << i);
//            BOOST_CHECK_MESSAGE(rbalanceMember_new_case3[2] == rbalanceMember_ret_case3[2], "case: " << i);
//        }

//    }

//    // Clean
//    {
//        pdb->ISNSqlDelete(tableClub, clubFieldAddress, testAddr);
//        pdb->ISNSqlDelete(tableMember, memFieldAddress, testAddr);
//        pdb->ISNSqlDelete(tableMember, memFieldAddress, memberAddr[0]);
//        pdb->ISNSqlDelete(tableMember, memFieldAddress, memberAddr[1]);
//        pdb->ISNSqlDelete(tableMember, memFieldAddress, memberAddr[2]);
//    }
//    // Stop ISNDB service
//    ISNDB::StopISNDBService();
//}

BOOST_AUTO_TEST_CASE(ComputeMemberReward_test)
{
    // Create inputs
    const uint txCntCase = 4;
    const uint totalTXCntCase = 4;
    const uint totalRewardsCase = 5;
    const uint64_t txCnt[txCntCase] = {0, 43251, 0x7fffffffffffffff, 0xffffffffffffffff};
    const uint64_t totalTXCnt[totalTXCntCase] = {0, 43251, 0x7fffffffffffffff, 0xffffffffffffffff};
    const CAmount totalRewards[totalRewardsCase] = {-1, 0, 20000, 0x7fffffffffffffff, 0x7fffffffffffffff+1};
    const uint testTime = txCntCase * totalTXCntCase * totalRewardsCase;

    // Create outputs
    bool expRet[testTime];
    CAmount expMemberReward[testTime] = {0};
    for(uint i = 0; i < txCntCase; i++)
    {
        for(uint j = 0; j < totalTXCntCase; j++)
        {
            for(uint k = 0; k < totalRewardsCase; k++)
            {
                uint num = k+totalRewardsCase*j+totalTXCntCase*totalRewardsCase*i;
                if (j == 0 || i > j || k == 0 || k >= 3)
                    expRet[num] = false;
                else
                {
                    expRet[num] = true;
                    arith_uint256 ttc = totalTXCnt[j];
                    arith_uint256 tc = txCnt[i];
                    expMemberReward[num] = tc.getdouble() / ttc.getdouble() * totalRewards[k];
                }
            }
        }
    }

    // Execute tests
    bool ret[testTime];
    CAmount retMemberReward[testTime] = {0};
    for(uint i = 0; i < txCntCase; i++)
    {
        for(uint j = 0; j < totalTXCntCase; j++)
        {
            for(uint k = 0; k < totalRewardsCase; k++)
            {
                uint num = k+totalRewardsCase*j+totalTXCntCase*totalRewardsCase*i;
                ret[num] = ComputeMemberReward(txCnt[i], totalTXCnt[j], totalRewards[k], retMemberReward[num]);
            }
        }
    }

    // Verify
    for(uint i = 0; i < txCntCase; i++)
    {
        for(uint j = 0; j < totalTXCntCase; j++)
        {
            for(uint k = 0; k < totalRewardsCase; k++)
            {
                uint num = k+totalRewardsCase*j+totalTXCntCase*totalRewardsCase*i;
                BOOST_CHECK_MESSAGE(ret[num] == expRet[num], "ret case: "<<i<<", "<<j<<", "<<k);
                if (ret[num])
                    BOOST_CHECK_MESSAGE(expMemberReward[num] == retMemberReward[num], "reward case: "<<i<<", "<<j<<", "<<k);
            }
        }
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

//BOOST_AUTO_TEST_CASE(RewardChangeUpdate_simulation_test)
//{
//    // Init
//    mapArgs["-mysqldbname"] = "imreward";
//    mapArgs["-mysqlserver"] = "localhost";
//    mapArgs["-mysqlusername"] = "root";
//    mapArgs["-mysqlpassword"] = "Mp3895";
//    // Here start ISNDB service ASAP
//    ISNDB::StartISNDBService();
//    // Construct DB
//    ISNDB* pdb = ISNDB::GetInstance();

//    // Create inputs
//    const uint rewardChangeCase = 7;
//    const uint addressCase = 1000;
//    const uint isUndoCase = 2;
//    const CAmount rewardChange[rewardChangeCase] = {0x7fffffffffffffff+1, -43251, 0, 43251, MAX_MONEY-1, 0x7fffffffffffffff, CAmount(0xffffffffffffffff)};
//    string address[addressCase];
//    const bool isUndo[isUndoCase] = {false, true};
//    const uint testTime = rewardChangeCase * addressCase * isUndoCase;
//    for(uint i = 0; i < addressCase; i++)
//    {
//        address[i] = GetRandomAddress();
//        AddNewLeader(pdb, address[i]);
//    }

//    // Create outputs
//    bool expRet[testTime];
//    for(uint i = 0; i < isUndoCase; i++)
//    {
//        for(uint j = 0; j < addressCase; j++)
//        {
//            for(uint k = 0; k < rewardChangeCase; k++)
//            {
//                uint num = k+rewardChangeCase*j+addressCase*rewardChangeCase*i;

//            }
//        }
//    }

//    // Execute tests
//    bool ret[testTime];
//    for(uint i = 0; i < isUndoCase; i++)
//    {
//        for(uint j = 0; j < addressCase; j++)
//        {
//            for(uint k = 0; k < rewardChangeCase; k++)
//            {
//                uint num = k+rewardChangeCase*j+addressCase*rewardChangeCase*i;
//                ret[num] = RewardChangeUpdate(rewardChange[k], address[j], isUndo[i]);
//            }
//        }
//    }

//    // Verify
//    for(uint i = 0; i < isUndoCase; i++)
//    {
//        for(uint j = 0; j < addressCase; j++)
//        {
//            for(uint k = 0; k < rewardChangeCase; k++)
//            {
//                uint num = k+rewardChangeCase*j+addressCase*rewardChangeCase*i;

//            }
//        }
//    }

//    // Clean
//    for(uint i = 0; i < addressCase; i++)
//    {
//        pdb->ISNSqlDelete(tableClub, clubFieldAddress, address[i]);
//    }
//    // Stop ISNDB service
//    ISNDB::StopISNDBService();

//    assert(false);
//}

BOOST_AUTO_TEST_CASE(RewardRateUpdate_simulation_test)
{
    // Init
    mapArgs["-updaterewardrate"] = "true";
    mapMultiArgs["-updaterewardrate"].push_back("true");
    if (prewardratedbview != NULL)
    {
        delete prewardratedbview;
        prewardratedbview = NULL;
    }
    prewardratedbview = new CRewardRateViewDB();

    // Create inputs
    const uint isUndoCase = 2;
    const uint addressCase = NUM_SIMULATION_ITERATIONS;
    const uint blockRewardCase = 7;
    const uint distributedRewardsCase = 7;
    const bool isUndo[isUndoCase] = {false, true};
    string address[addressCase];
    const CAmount blockReward[blockRewardCase] = {
        0x7fffffffffffffff+1, -43251, 0, 43251, MAX_MONEY-1, 0x7fffffffffffffff, CAmount(0xffffffffffffffff)
    };
    const CAmount distributedRewards[distributedRewardsCase] = {
        0x7fffffffffffffff+1, -43251, 0, 43251, MAX_MONEY-1, 0x7fffffffffffffff, CAmount(0xffffffffffffffff)
    };
    const uint testTime = isUndoCase * addressCase;
    for(uint i = 0; i < addressCase; i++)
        address[i] = GetRandomAddress();

    // Create outputs

    // Execute tests
    bool ret[testTime];
    string expAddress[testTime];
    for(uint i = 0; i < 1/*isUndoCase*/; i++)
    {
        for(uint j = 0; j < addressCase; j++)
        {
            for(uint k = 0; k < 1; k++)
            {
                //uint num = k+rewardChangeCase*j+addressCase*rewardChangeCase*i;
                assert(RewardRateUpdate(blockReward[4], distributedRewards[3], address[j], j, isUndo[i]));
                //ret[j] = RewardRateUpdate(blockReward[4], distributedRewards[3], address[j], j, isUndo[i]);
            }
        }
    }

    // Verify
    for(uint j = 0; j < addressCase; j++)
    {
        ret[j] = prewardratedbview->GetRewardRate(j, expAddress[j]);
    }

    // Clean
    if (prewardratedbview != NULL)
    {
        delete prewardratedbview;
        prewardratedbview = NULL;
    }

    assert(false);
}

// This test is similar to the previous test
// except the emphasis is on testing the functionality of UpdateCoins
// random txs are created and UpdateCoins is used to update the cache stack
// In particular it is tested that spending a duplicate coinbase tx
// has the expected effect (the other duplicate is overwitten at all cache levels)
BOOST_AUTO_TEST_CASE(updatecoins_simulation_test)
{
    bool spent_a_duplicate_coinbase = false;
    // A simple map to track what we expect the cache stack to represent.
    std::map<uint256, CCoins> result;

    // The cache stack.
    CCoinsViewTest base; // A CCoinsViewTest at the bottom.
    std::vector<CCoinsViewCacheTest*> stack; // A stack of CCoinsViewCaches on top.
    stack.push_back(new CCoinsViewCacheTest(&base)); // Start with one cache.

    // Track the txids we've used and whether they have been spent or not
    std::map<uint256, CAmount> coinbaseids;
    std::set<uint256> alltxids;
    std::set<uint256> duplicateids;

    for (unsigned int i = 0; i < NUM_SIMULATION_ITERATIONS; i++) {
        {
            CMutableTransaction tx;
            tx.vin.resize(1);
            tx.vout.resize(1);
            tx.vout[0].nValue = i; //Keep txs unique unless intended to duplicate
            unsigned int height = insecure_rand();

            // 1/10 times create a coinbase
            if (insecure_rand() % 10 == 0 || coinbaseids.size() < 10) {
                // 1/100 times create a duplicate coinbase
                if (insecure_rand() % 10 == 0 && coinbaseids.size()) {
                    std::map<uint256, CAmount>::iterator coinbaseIt = coinbaseids.lower_bound(GetRandHash());
                    if (coinbaseIt == coinbaseids.end()) {
                        coinbaseIt = coinbaseids.begin();
                    }
                    //Use same random value to have same hash and be a true duplicate
                    tx.vout[0].nValue = coinbaseIt->second;
                    assert(tx.GetHash() == coinbaseIt->first);
                    duplicateids.insert(coinbaseIt->first);
                }
                else {
                    coinbaseids[tx.GetHash()] = tx.vout[0].nValue;
                }
                assert(CTransaction(tx).IsCoinBase());
            }
            // 9/10 times create a regular tx
            else {
                uint256 prevouthash;
                // equally likely to spend coinbase or non coinbase
                std::set<uint256>::iterator txIt = alltxids.lower_bound(GetRandHash());
                if (txIt == alltxids.end()) {
                    txIt = alltxids.begin();
                }
                prevouthash = *txIt;

                // Construct the tx to spend the coins of prevouthash
                tx.vin[0].prevout.hash = prevouthash;
                tx.vin[0].prevout.n = 0;

                // Update the expected result of prevouthash to know these coins are spent
                CCoins& oldcoins = result[prevouthash];
                oldcoins.Clear();

                // It is of particular importance here that once we spend a coinbase tx hash
                // it is no longer available to be duplicated (or spent again)
                // BIP 34 in conjunction with enforcing BIP 30 (at least until BIP 34 was active)
                // results in the fact that no coinbases were duplicated after they were already spent
                alltxids.erase(prevouthash);
                coinbaseids.erase(prevouthash);

                // The test is designed to ensure spending a duplicate coinbase will work properly
                // if that ever happens and not resurrect the previously overwritten coinbase
                if (duplicateids.count(prevouthash))
                    spent_a_duplicate_coinbase = true;

                assert(!CTransaction(tx).IsCoinBase());
            }
            // Track this tx to possibly spend later
            alltxids.insert(tx.GetHash());

            // Update the expected result to know about the new output coins
            CCoins &coins = result[tx.GetHash()];
            coins.FromTx(tx, height);

            UpdateCoins(tx, *(stack.back()), height);
        }

        // Once every 1000 iterations and at the end, verify the full cache.
        if (insecure_rand() % 1000 == 1 || i == NUM_SIMULATION_ITERATIONS - 1) {
            for (std::map<uint256, CCoins>::iterator it = result.begin(); it != result.end(); it++) {
                const CCoins* coins = stack.back()->AccessCoins(it->first);
                if (coins) {
                    BOOST_CHECK(*coins == it->second);
                 } else {
                    BOOST_CHECK(it->second.IsPruned());
                 }
            }
        }

        if (insecure_rand() % 100 == 0) {
            // Every 100 iterations, flush an intermediate cache
            if (stack.size() > 1 && insecure_rand() % 2 == 0) {
                unsigned int flushIndex = insecure_rand() % (stack.size() - 1);
                stack[flushIndex]->Flush();
            }
        }
        if (insecure_rand() % 100 == 0) {
            // Every 100 iterations, change the cache stack.
            if (stack.size() > 0 && insecure_rand() % 2 == 0) {
                stack.back()->Flush();
                delete stack.back();
                stack.pop_back();
            }
            if (stack.size() == 0 || (stack.size() < 4 && insecure_rand() % 2)) {
                CCoinsView* tip = &base;
                if (stack.size() > 0) {
                    tip = stack.back();
                }
                stack.push_back(new CCoinsViewCacheTest(tip));
           }
        }
    }

    // Clean up the stack.
    while (stack.size() > 0) {
        delete stack.back();
        stack.pop_back();
    }

    // Verify coverage.
    BOOST_CHECK(spent_a_duplicate_coinbase);
}

BOOST_AUTO_TEST_CASE(ccoins_serialization)
{
    // Good example
    CDataStream ss1(ParseHex("0104835800816115944e077fe7c803cfa57f29b36bf87c1d358bb85e"), SER_DISK, CLIENT_VERSION);
    CCoins cc1;
    ss1 >> cc1;
    BOOST_CHECK_EQUAL(cc1.nVersion, 1);
    BOOST_CHECK_EQUAL(cc1.fCoinBase, false);
    BOOST_CHECK_EQUAL(cc1.nHeight, 203998);
    BOOST_CHECK_EQUAL(cc1.vout.size(), 2);
    BOOST_CHECK_EQUAL(cc1.IsAvailable(0), false);
    BOOST_CHECK_EQUAL(cc1.IsAvailable(1), true);
    BOOST_CHECK_EQUAL(cc1.vout[1].nValue, 60000000000ULL);
    BOOST_CHECK_EQUAL(HexStr(cc1.vout[1].scriptPubKey), HexStr(GetScriptForDestination(CKeyID(uint160(ParseHex("816115944e077fe7c803cfa57f29b36bf87c1d35"))))));

    // Good example
    CDataStream ss2(ParseHex("0109044086ef97d5790061b01caab50f1b8e9c50a5057eb43c2d9563a4eebbd123008c988f1a4a4de2161e0f50aac7f17e7f9555caa486af3b"), SER_DISK, CLIENT_VERSION);
    CCoins cc2;
    ss2 >> cc2;
    BOOST_CHECK_EQUAL(cc2.nVersion, 1);
    BOOST_CHECK_EQUAL(cc2.fCoinBase, true);
    BOOST_CHECK_EQUAL(cc2.nHeight, 120891);
    BOOST_CHECK_EQUAL(cc2.vout.size(), 17);
    for (int i = 0; i < 17; i++) {
        BOOST_CHECK_EQUAL(cc2.IsAvailable(i), i == 4 || i == 16);
    }
    BOOST_CHECK_EQUAL(cc2.vout[4].nValue, 234925952);
    BOOST_CHECK_EQUAL(HexStr(cc2.vout[4].scriptPubKey), HexStr(GetScriptForDestination(CKeyID(uint160(ParseHex("61b01caab50f1b8e9c50a5057eb43c2d9563a4ee"))))));
    BOOST_CHECK_EQUAL(cc2.vout[16].nValue, 110397);
    BOOST_CHECK_EQUAL(HexStr(cc2.vout[16].scriptPubKey), HexStr(GetScriptForDestination(CKeyID(uint160(ParseHex("8c988f1a4a4de2161e0f50aac7f17e7f9555caa4"))))));

    // Smallest possible example
    CDataStream ssx(SER_DISK, CLIENT_VERSION);
    BOOST_CHECK_EQUAL(HexStr(ssx.begin(), ssx.end()), "");

    CDataStream ss3(ParseHex("0002000600"), SER_DISK, CLIENT_VERSION);
    CCoins cc3;
    ss3 >> cc3;
    BOOST_CHECK_EQUAL(cc3.nVersion, 0);
    BOOST_CHECK_EQUAL(cc3.fCoinBase, false);
    BOOST_CHECK_EQUAL(cc3.nHeight, 0);
    BOOST_CHECK_EQUAL(cc3.vout.size(), 1);
    BOOST_CHECK_EQUAL(cc3.IsAvailable(0), true);
    BOOST_CHECK_EQUAL(cc3.vout[0].nValue, 0);
    BOOST_CHECK_EQUAL(cc3.vout[0].scriptPubKey.size(), 0);

    // scriptPubKey that ends beyond the end of the stream
    CDataStream ss4(ParseHex("0002000800"), SER_DISK, CLIENT_VERSION);
    try {
        CCoins cc4;
        ss4 >> cc4;
        BOOST_CHECK_MESSAGE(false, "We should have thrown");
    } catch (const std::ios_base::failure& e) {
    }

    // Very large scriptPubKey (3*10^9 bytes) past the end of the stream
    CDataStream tmp(SER_DISK, CLIENT_VERSION);
    uint64_t x = 3000000000ULL;
    tmp << VARINT(x);
    BOOST_CHECK_EQUAL(HexStr(tmp.begin(), tmp.end()), "8a95c0bb00");
    CDataStream ss5(ParseHex("0002008a95c0bb0000"), SER_DISK, CLIENT_VERSION);
    try {
        CCoins cc5;
        ss5 >> cc5;
        BOOST_CHECK_MESSAGE(false, "We should have thrown");
    } catch (const std::ios_base::failure& e) {
    }
}

BOOST_AUTO_TEST_SUITE_END()
