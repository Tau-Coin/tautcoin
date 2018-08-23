// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txdb.h"

#include "arith_uint256.h"
#include "chainparams.h"
#include "hash.h"
#include "main.h"
#include "uint256.h"

#include <stdint.h>

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

static const char DB_COINS = 'c';
static const char DB_COINS_BYSCRIPT = 's';
static const char DB_BLOCK_FILES = 'f';
static const char DB_TXINDEX = 't';
static const char DB_BLOCK_INDEX = 'b';

static const char DB_BEST_BLOCK = 'B';
static const char DB_FLAG = 'F';
static const char DB_REINDEX_FLAG = 'R';
static const char DB_LAST_BLOCK = 'l';


CCoinsViewDB::CCoinsViewDB(size_t nCacheSize, bool fMemory, bool fWipe) : db(GetDataDir() / "chainstate", nCacheSize, fMemory, fWipe, true) 
{
    pcoinsViewByScript = NULL;
}

void CCoinsViewDB::SetCoinsViewByScript(CCoinsViewByScript* pcoinsViewByScriptIn) {
    pcoinsViewByScript = pcoinsViewByScriptIn;
}

bool CCoinsViewDB::GetCoins(const uint256 &txid, CCoins &coins) const {
    return db.Read(make_pair(DB_COINS, txid), coins);
}

bool CCoinsViewDB::GetCoinsByHashOfScript(const uint256 &hash, CCoinsByScript &coins) const {
    return db.Read(make_pair(DB_COINS_BYSCRIPT, hash), coins);
}

bool CCoinsViewDB::HaveCoins(const uint256 &txid) const {
    return db.Exists(make_pair(DB_COINS, txid));
}

uint256 CCoinsViewDB::GetBestBlock() const {
    uint256 hashBestChain;
    if (!db.Read(DB_BEST_BLOCK, hashBestChain))
        return uint256();
    return hashBestChain;
}

bool CCoinsViewDB::BatchWrite(CCoinsMap &mapCoins, const uint256 &hashBlock) {
    CDBBatch batch(db);
    size_t count = 0;
    size_t changed = 0;
    for (CCoinsMap::iterator it = mapCoins.begin(); it != mapCoins.end();) {
        if (it->second.flags & CCoinsCacheEntry::DIRTY) {
            if (it->second.coins.IsPruned())
                batch.Erase(make_pair(DB_COINS, it->first));
            else
                batch.Write(make_pair(DB_COINS, it->first), it->second.coins);
            changed++;
        }
        count++;
        CCoinsMap::iterator itOld = it++;
        mapCoins.erase(itOld);
    }

    if (pcoinsViewByScript) // only if -txoutsbyaddressindex
    {
        for (CCoinsMapByScript::iterator it = pcoinsViewByScript->cacheCoinsByScript.begin(); it != pcoinsViewByScript->cacheCoinsByScript.end();) {
            BatchWriteCoinsByScript(batch, it->first, it->second);
            CCoinsMapByScript::iterator itOld = it++;
            pcoinsViewByScript->cacheCoinsByScript.erase(itOld);
        }
        pcoinsViewByScript->cacheCoinsByScript.clear();
    }

    if (!hashBlock.IsNull())
        batch.Write(DB_BEST_BLOCK, hashBlock);

    LogPrint("coindb", "Committing %u changed transactions (out of %u) to coin database...\n", (unsigned int)changed, (unsigned int)count);
    return db.WriteBatch(batch);
}

bool CCoinsViewDB::BatchWriteCoinsByScript(CDBBatch& batch, const uint256 &hash, const CCoinsByScript &coins) {
    if (coins.IsEmpty())
        batch.Erase(make_pair(DB_COINS_BYSCRIPT, hash));
    else
        batch.Write(make_pair(DB_COINS_BYSCRIPT, hash), coins);

    return true;
}

bool CCoinsViewDB::GetStats(CCoinsStats &stats) const {
    /* It seems that there are no "const iterators" for LevelDB.  Since we
       only need read operations on it, use a const-cast to get around
       that restriction.  */
    boost::scoped_ptr<CDBIterator> pcursor(const_cast<CDBWrapper*>(&db)->NewIterator());
    pcursor->SeekToFirst();

    CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
    stats.hashBlock = GetBestBlock();
    ss << stats.hashBlock;
    CAmount nTotalAmount = 0;
    std::pair<char, uint256> key;
    char chType;

    while (pcursor->Valid() && pcursor->GetKey(key)) {
        boost::this_thread::interruption_point();
        try {
            chType = key.first;
            LogPrintf("%s read %c type from coindb...\n", __func__, chType);

            if (chType == DB_COINS) {
                CCoins coins;
                if (!pcursor->GetValue(coins)) {
                    LogPrintf("%s: unable to read value for type %c", __func__, DB_COINS);
                    break;
                }
                uint256 txhash = key.second;
                ss << txhash;
                ss << VARINT(coins.nVersion);
                ss << (coins.fCoinBase ? 'c' : 'n');
                ss << VARINT(coins.nHeight);
                stats.nTransactions++;
                for (unsigned int i = 0; i< coins.vout.size(); i++) {
                    const CTxOut &out = coins.vout[i];
                    if (!out.IsNull()) {
                        stats.nTransactionOutputs++;
                        ss << VARINT(i+1);
                        ss << out;
                        nTotalAmount += out.nValue;
                    }
                }
                stats.nSerializedSize += 32 + pcursor->GetValueSize();
                ss << VARINT(0);
            }
            if (chType == DB_COINS_BYSCRIPT) {
                CCoinsByScript coinsByScript;
                if (!pcursor->GetValue(coinsByScript)) {
                    LogPrintf("%s: unable to read value for type %c", __func__, DB_COINS_BYSCRIPT);
                    break;
                }
                stats.nAddresses++;
                stats.nAddressesOutputs += coinsByScript.setCoins.size();
            }
            pcursor->Next();
        } catch (const std::exception& e) {
            return error("%s: Deserialize or I/O error - %s", __func__, e.what());
        }
    }

    {
        LOCK(cs_main);
        if (mapBlockIndex.count(GetBestBlock()))
            stats.nHeight = mapBlockIndex.find(GetBestBlock())->second->nHeight;
    }
    stats.hashSerialized = ss.GetHash();
    stats.nTotalAmount = nTotalAmount;

    return true;
}

bool CCoinsViewDB::DeleteAllCoinsByScript()
{
    LogPrintf("Delete address index for -txoutsbyaddressindex. Be patient...\n");
    boost::scoped_ptr<CDBIterator> pcursor(const_cast<CDBWrapper*>(&db)->NewIterator());
    CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
    //ssKeySet << DB_COINS_BYSCRIPT;
    pcursor->Seek(DB_COINS_BYSCRIPT);

    std::vector<uint256> v;
    int64_t i = 0;
    std::pair<char, uint256> key;
    char chType;

    while (pcursor->Valid() && pcursor->GetKey(key)) {
        boost::this_thread::interruption_point();
        try {
            chType = key.first;
            LogPrintf("%s read %c type from coindb...\n", __func__, chType);

            if (chType != DB_COINS_BYSCRIPT)
                break;

            uint256 scripthash = key.second;
            v.push_back(scripthash);
            if (v.size() >= 10000)
            {
                i += v.size();
                CDBBatch batch(db);
                CCoinsByScript empty;
                BOOST_FOREACH(const uint256& hash, v)
                    BatchWriteCoinsByScript(batch, hash, empty); // delete
                db.WriteBatch(batch);
                v.clear();
            }

            pcursor->Next();
        } catch (std::exception &e) {
            return error("%s : Deserialize or I/O error - %s", __func__, e.what());
        }
    }
    if (!v.empty())
    {
        i += v.size();
        CDBBatch batch(db);
        CCoinsByScript empty;
        BOOST_FOREACH(const uint256& hash, v)
            BatchWriteCoinsByScript(batch, hash, empty); // delete
        db.WriteBatch(batch);
    }
    if (i > 0)
        LogPrintf("Address index with %d addresses successfully deleted.\n", i);

    return true;
}

bool CCoinsViewDB::GenerateAllCoinsByScript()
{
    LogPrintf("Building address index for -txoutsbyaddressindex. Be patient...\n");

    boost::scoped_ptr<CDBIterator> pcursor(const_cast<CDBWrapper*>(&db)->NewIterator());
    CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
    //ssKeySet << DB_COINS;
    pcursor->Seek(DB_COINS);

    CCoinsMapByScript mapCoinsByScript;
    int64_t i = 0;
    std::pair<char, uint256> key;
    char chType;

    while (pcursor->Valid() && pcursor->GetKey(key)) {
        boost::this_thread::interruption_point();
        try {
            chType = key.first;
            LogPrintf("%s read %c type from coindb...\n", __func__, chType);

            if (chType != DB_COINS)
                break;

            CCoins coins;
            if (!pcursor->GetValue(coins)) {
                LogPrintf("%s: unable to read value for type %c", __func__, DB_COINS);
                break;
            }

            uint256 txhash = key.second;

            for (unsigned int j = 0; j < coins.vout.size(); j++)
            {
                if (coins.vout[j].IsNull() || coins.vout[j].scriptPubKey.IsUnspendable())
                    continue;

                const uint256 key = CCoinsViewByScript::getKey(coins.vout[j].scriptPubKey);
                if (!mapCoinsByScript.count(key))
                {
                    CCoinsByScript coinsByScript;
                    GetCoinsByHashOfScript(key, coinsByScript);
                    mapCoinsByScript.insert(make_pair(key, coinsByScript));
                }
                mapCoinsByScript[key].setCoins.insert(COutPoint(txhash, (uint32_t)j));
                i++;
            }

            if (mapCoinsByScript.size() >= 10000)
            {
                CDBBatch batch(db);
                for (CCoinsMapByScript::iterator it = mapCoinsByScript.begin(); it != mapCoinsByScript.end();) {
                    BatchWriteCoinsByScript(batch, it->first, it->second);
                    CCoinsMapByScript::iterator itOld = it++;
                    mapCoinsByScript.erase(itOld);
                }
                db.WriteBatch(batch);
                mapCoinsByScript.clear();
            }

            pcursor->Next();
        } catch (std::exception &e) {
            return error("%s : Deserialize or I/O error - %s", __func__, e.what());
        }
    }
    if (!mapCoinsByScript.empty())
    {
       CDBBatch batch(db);
       for (CCoinsMapByScript::iterator it = mapCoinsByScript.begin(); it != mapCoinsByScript.end();) {
           BatchWriteCoinsByScript(batch, it->first, it->second);
           CCoinsMapByScript::iterator itOld = it++;
           mapCoinsByScript.erase(itOld);
       }
       db.WriteBatch(batch);
    }
    LogPrintf("Address index with %d outputs successfully built.\n", i);
    return true;
}

CBlockTreeDB::CBlockTreeDB(size_t nCacheSize, bool fMemory, bool fWipe) : CDBWrapper(GetDataDir() / "blocks" / "index", nCacheSize, fMemory, fWipe) {
}

bool CBlockTreeDB::ReadBlockFileInfo(int nFile, CBlockFileInfo &info) {
    return Read(make_pair(DB_BLOCK_FILES, nFile), info);
}

bool CBlockTreeDB::WriteReindexing(bool fReindexing) {
    if (fReindexing)
        return Write(DB_REINDEX_FLAG, '1');
    else
        return Erase(DB_REINDEX_FLAG);
}

bool CBlockTreeDB::ReadReindexing(bool &fReindexing) {
    fReindexing = Exists(DB_REINDEX_FLAG);
    return true;
}

bool CBlockTreeDB::ReadLastBlockFile(int &nFile) {
    return Read(DB_LAST_BLOCK, nFile);
}

CCoinsViewCursor *CCoinsViewDB::Cursor() const
{
    CCoinsViewDBCursor *i = new CCoinsViewDBCursor(const_cast<CDBWrapper*>(&db)->NewIterator(), GetBestBlock());
    /* It seems that there are no "const iterators" for LevelDB.  Since we
       only need read operations on it, use a const-cast to get around
       that restriction.  */
    i->pcursor->Seek(DB_COINS);
    // Cache key of first record
    i->pcursor->GetKey(i->keyTmp);
    return i;
}

bool CCoinsViewDBCursor::GetKey(uint256 &key) const
{
    // Return cached key
    if (keyTmp.first == DB_COINS) {
        key = keyTmp.second;
        return true;
    }
    return false;
}

bool CCoinsViewDBCursor::GetValue(CCoins &coins) const
{
    return pcursor->GetValue(coins);
}

unsigned int CCoinsViewDBCursor::GetValueSize() const
{
    return pcursor->GetValueSize();
}

bool CCoinsViewDBCursor::Valid() const
{
    return keyTmp.first == DB_COINS;
}

void CCoinsViewDBCursor::Next()
{
    pcursor->Next();
    if (!pcursor->Valid() || !pcursor->GetKey(keyTmp))
        keyTmp.first = 0; // Invalidate cached key after last record so that Valid() and GetKey() return false
}

bool CBlockTreeDB::WriteBatchSync(const std::vector<std::pair<int, const CBlockFileInfo*> >& fileInfo, int nLastFile, const std::vector<const CBlockIndex*>& blockinfo) {
    CDBBatch batch(*this);
    for (std::vector<std::pair<int, const CBlockFileInfo*> >::const_iterator it=fileInfo.begin(); it != fileInfo.end(); it++) {
        batch.Write(make_pair(DB_BLOCK_FILES, it->first), *it->second);
    }
    batch.Write(DB_LAST_BLOCK, nLastFile);
    for (std::vector<const CBlockIndex*>::const_iterator it=blockinfo.begin(); it != blockinfo.end(); it++) {
        batch.Write(make_pair(DB_BLOCK_INDEX, (*it)->GetBlockHash()), CDiskBlockIndex(*it));
    }
    return WriteBatch(batch, true);
}

bool CBlockTreeDB::ReadTxIndex(const uint256 &txid, CDiskTxPos &pos) {
    return Read(make_pair(DB_TXINDEX, txid), pos);
}

bool CBlockTreeDB::WriteTxIndex(const std::vector<std::pair<uint256, CDiskTxPos> >&vect) {
    CDBBatch batch(*this);
    for (std::vector<std::pair<uint256,CDiskTxPos> >::const_iterator it=vect.begin(); it!=vect.end(); it++)
        batch.Write(make_pair(DB_TXINDEX, it->first), it->second);
    return WriteBatch(batch);
}

bool CBlockTreeDB::WriteFlag(const std::string &name, bool fValue) {
    return Write(std::make_pair(DB_FLAG, name), fValue ? '1' : '0');
}

bool CBlockTreeDB::ReadFlag(const std::string &name, bool &fValue) {
    char ch;
    if (!Read(std::make_pair(DB_FLAG, name), ch))
        return false;
    fValue = ch == '1';
    return true;
}

bool CBlockTreeDB::LoadBlockIndexGuts(boost::function<CBlockIndex*(const uint256&)> insertBlockIndex)
{
    boost::scoped_ptr<CDBIterator> pcursor(NewIterator());

    pcursor->Seek(make_pair(DB_BLOCK_INDEX, uint256()));

    // Load mapBlockIndex
    while (pcursor->Valid()) {
        boost::this_thread::interruption_point();
        std::pair<char, uint256> key;
        if (pcursor->GetKey(key) && key.first == DB_BLOCK_INDEX) {
            CDiskBlockIndex diskindex;
            if (pcursor->GetValue(diskindex)) {
                // Construct block index object
                CBlockIndex* pindexNew = insertBlockIndex(diskindex.GetBlockHash());
                pindexNew->pprev          = insertBlockIndex(diskindex.hashPrev);
                pindexNew->nHeight        = diskindex.nHeight;
                pindexNew->nFile          = diskindex.nFile;
                pindexNew->nDataPos       = diskindex.nDataPos;
                pindexNew->nUndoPos       = diskindex.nUndoPos;
                pindexNew->nVersion       = diskindex.nVersion;
                pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
                pindexNew->nTime          = diskindex.nTime;
                pindexNew->nStatus        = diskindex.nStatus;
                pindexNew->nTx            = diskindex.nTx;

                pindexNew->baseTarget                  = diskindex.baseTarget;
                pindexNew->generationSignature         = diskindex.generationSignature;
                pindexNew->harvestPower                = diskindex.harvestPower;
                pindexNew->pubKeyOfpackager            = diskindex.pubKeyOfpackager;
                pindexNew->cumulativeDifficulty        = diskindex.cumulativeDifficulty;
                pindexNew->nChainDiff        = UintToArith256(diskindex.cumulativeDifficulty);

                pcursor->Next();
            } else {
                return error("LoadBlockIndex() : failed to read value");
            }
        } else {
            break;
        }
    }

    return true;
}

CRwdBalanceViewDB::CRwdBalanceViewDB()
{
    options.create_if_missing = true;

    std::string db_path = GetDataDir(true).string() + std::string(RWDBLDBPATH);
    LogPrintf("Opening LevelDB in %s\n", db_path);

    leveldb::Status status = leveldb::DB::Open(options, db_path, &pdb);
    dbwrapper_private::HandleError(status);
    assert(status.ok());
    LogPrintf("Opened LevelDB successfully\n");
}

CRwdBalanceViewDB::~CRwdBalanceViewDB()
{
    delete pdb;
    pdb = NULL;
}

bool CRwdBalanceViewDB::WriteDB(std::string key, int nHeight, string strValue)
{
    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Put(leveldb::WriteOptions(), key+DBSEPECTATOR+strHeight, strValue);
    if(!status.ok())
    {
        LogPrintf("LevelDB write failure in rwdbalance module: %s\n", status.ToString());
        dbwrapper_private::HandleError(status);
        return false;
    }

    return true;
}

bool CRwdBalanceViewDB::WriteDB(std::string key, int nHeight, string father, uint64_t tc, CAmount value)
{
    return WriteDB(key, nHeight, GenerateRecord(father, tc, value));
}

bool CRwdBalanceViewDB::ReadDB(std::string key, int nHeight, string& father, uint64_t& tc, CAmount& value)
{
    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    std::string strValue;
    leveldb::Status status = pdb->Get(leveldb::ReadOptions(), key+DBSEPECTATOR+strHeight, &strValue);
    if(!status.ok())
    {
        if (status.IsNotFound())
        {
            father = " ";
            tc = 0;
            value = 0;
        }
        else
        {
            LogPrintf("LevelDB read failure in rwdbalance module: %s\n", status.ToString());
            dbwrapper_private::HandleError(status);
        }
        return false;
    }

    if (!ParseRecord(strValue, father, tc, value))
        return false;

    return true;
}

bool CRwdBalanceViewDB::ReadDB(std::string key, int nHeight, std::string& strValue)
{
    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Get(leveldb::ReadOptions(), key+DBSEPECTATOR+strHeight, &strValue);
    if(!status.ok())
    {
        if (status.IsNotFound())
            strValue = " _0_0";
        else
        {
            LogPrintf("LevelDB read failure in rwdbalance module: %s\n", status.ToString());
            dbwrapper_private::HandleError(status);
        }
        return false;
    }

    return true;
}

bool CRwdBalanceViewDB::DeleteDB(std::string key, int nHeight)
{
    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Delete(leveldb::WriteOptions(), key+DBSEPECTATOR+strHeight);
    if(!status.ok() && !status.IsNotFound())
    {
        LogPrintf("LevelDB write failure in rwdbalance module: %s\n", status.ToString());
        dbwrapper_private::HandleError(status);
        return false;
    }

    return true;
}

void CRwdBalanceViewDB::ClearCache()
{
    cacheRecord.clear();
}

void CRwdBalanceViewDB::UpdateCacheFather(string address, int inputHeight, string newFather)
{
    CAmount rwdbalanceInput = 0;
    string ftInput = " ";
    uint64_t tcInput = 1;
    GetFullRecord(address, inputHeight, ftInput, tcInput, rwdbalanceInput);
    ftInput = newFather;
    string newRecordInput = GenerateRecord(ftInput, tcInput, rwdbalanceInput);
    cacheRecord[address] = newRecordInput;
}

void CRwdBalanceViewDB::UpdateCacheTcAddOne(string address, int inputHeight)
{
    CAmount rwdbalanceVout = 0;
    string ftVout = " ";
    uint64_t tcVout = 1;
    GetFullRecord(address, inputHeight, ftVout, tcVout, rwdbalanceVout);
    tcVout++;
    string newRecordVout = GenerateRecord(ftVout, tcVout, rwdbalanceVout);
    cacheRecord[address] = newRecordVout;
}

void CRwdBalanceViewDB::UpdateCacheRewardChange(string address, int inputHeight, CAmount rewardChange)
{
    CAmount rewardbalance_old = 0;
    string ft = " ";
    uint64_t tc = 1;
    GetFullRecord(address, inputHeight, ft, tc, rewardbalance_old);
    CAmount newValue = rewardbalance_old + rewardChange;
    string newRecord = GenerateRecord(ft, tc, newValue);
    cacheRecord[address] = newRecord;
}

bool CRwdBalanceViewDB::Commit(int nHeight)
{
    if (cacheRecord.size() == 0)
        return true;

    for(std::map<string, string>::const_iterator it = cacheRecord.begin();
        it != cacheRecord.end(); it++)
    {
        string address = it->first;
        string strValue = it->second;
        if (!WriteDB(address, nHeight, strValue))
            return false;
    }

    return true;
}

bool CRwdBalanceViewDB::InitGenesisDB(std::vector<std::string> addresses)
{
    for(uint i = 0; i < addresses.size(); i++)
    {
        CAmount rewardbalance = 0;
        string ft = "0";
        uint64_t tc = 1;
        if (!WriteDB(addresses[i], 0, ft, tc, rewardbalance))
            return false;
    }
    return true;
}

bool CRwdBalanceViewDB::ParseRecord(string inputStr, string& father, uint64_t& tc, CAmount& value)
{
    vector<string> splitedStr;
    boost::split(splitedStr, inputStr, boost::is_any_of(DBSEPECTATOR));
    if (splitedStr.size() != 3)
        return false;
    father = splitedStr[0];
    std::istringstream ssVal1(splitedStr[1]);
    ssVal1 >> tc;
    std::istringstream ssVal2(splitedStr[2]);
    ssVal2 >> value;

    return true;
}

string CRwdBalanceViewDB::GenerateRecord(string father, uint64_t tc, CAmount value)
{
    string outputStr;
    std::stringstream ssVal;
    ssVal << father;
    ssVal << DBSEPECTATOR;
    ssVal << tc;
    ssVal << DBSEPECTATOR;
    ssVal << value;
    ssVal >> outputStr;

    return outputStr;
}

CAmount CRwdBalanceViewDB::GetRwdBalance(std::string address, int nHeight)
{
    string ft = " ";
    uint64_t tc;
    CAmount value = 0;
    GetFullRecord(address, nHeight, ft, tc, value);
    return value;
}

string CRwdBalanceViewDB::GetFather(string address, int nHeight)
{
    string ft = " ";
    uint64_t tc;
    CAmount value;
    GetFullRecord(address, nHeight, ft, tc, value);
    return ft;
}

uint64_t CRwdBalanceViewDB::GetTXCnt(string address, int nHeight)
{
    string ft;
    uint64_t tc = 0;
    CAmount value;
    GetFullRecord(address, nHeight, ft, tc, value);
    return tc;
}

void CRwdBalanceViewDB::GetFullRecord(string address, int nHeight, string& father, uint64_t& tc, CAmount& value)
{
    if (cacheRecord.find(address) != cacheRecord.end())
    {
        ParseRecord(cacheRecord[address], father, tc, value);
        return;
    }
    else
    {
        for (int h = nHeight; h >= 0; h--)
        {
            if (ReadDB(address, h, father, tc, value))
                return;
        }
    }
}

string CRwdBalanceViewDB::GetFullRecord(std::string address, int nHeight)
{
    string strValue;
    if (cacheRecord.find(address) != cacheRecord.end())
    {
        strValue = cacheRecord[address];
        return strValue;
    }
    else
    {
        for (int h = nHeight; h >= 0; h--)
        {
            if (ReadDB(address, h, strValue))
                return strValue;
        }
    }

    return strValue;
}

bool CRwdBalanceViewDB::RewardChangeUpdate(CAmount rewardChange, string address, int nHeight, bool isUndo)
{
    if (rewardChange >= MAX_MONEY || rewardChange <= -MAX_MONEY)
        return false;
    if (isUndo)
    {
        if (!DeleteDB(address, nHeight+1))
            return false;
        return true;
    }

    UpdateCacheRewardChange(address, nHeight-1, rewardChange);

    //std::cout<<"====="<<address<<":   "<<rewardbalance_old<<" + "<<rewardChange;
    //std::cout<<" = "<<newValue<<std::endl;

    //if (!WriteDB(address, nHeight, ft, tc, newValue))
    //    return false;
    //LogPrintf("%s, member:%s, rw:%d\n", __func__, member, rewardbalance_old);

    return true;
}

bool CRwdBalanceViewDB::RewardChangeUpdateByPubkey(CAmount rewardChange, string pubKey, int nHeight, bool isUndo)
{
    string address;

    if (pubKey.empty())
        return false;
    const CScript script = CScript() << ParseHex(pubKey) << OP_CHECKSIG;
    CBitcoinAddress addr;
    if (!addr.ScriptPub2Addr(script, address))
        return false;

    if (!RewardChangeUpdate(rewardChange, address, nHeight, isUndo))
        return false;

    return true;
}

bool CRwdBalanceViewDB::UpdateRewardsByTX(const CTransaction& tx, CAmount blockReward, int nHeight, bool isUndo)
{
    if (!tx.IsCoinBase())
    {
        for(unsigned int i = 0; i < tx.vreward.size(); i++)
        {
            if (!RewardChangeUpdateByPubkey(0-tx.vreward[i].rewardBalance,
                                            tx.vreward[i].senderPubkey, nHeight, isUndo))
                return false;
        }
        return true;
    }

    // Init rewards distribution and check if valid
    if (tx.vout.size() != 1)
    {
        LogPrintf("Error: The TX's vout size is 0\n");
        return false;
    }
    bool ret = true;
    CAmount distributedRewards = 0;
    string clubLeaderAddress;
    map<string, CAmount> memberRewards;
    CAmount memberTotalRewards = blockReward-tx.vout[0].nValue;
    if (!InitRewardsDist(memberTotalRewards, tx.vout[0].scriptPubKey, clubLeaderAddress,
                         distributedRewards, memberRewards))
        return false;

    // Distribute rewards to member and return remained rewards back to club leader
    for(std::map<string, CAmount>::const_iterator it = memberRewards.begin(); it != memberRewards.end(); it++)
        ret &= RewardChangeUpdate(it->second, it->first, nHeight, isUndo);
    CAmount remainedReward = memberTotalRewards - distributedRewards;
    ret &= RewardChangeUpdate(remainedReward, clubLeaderAddress, nHeight, isUndo);

    // Update rewards rate
    RewardRateUpdate(blockReward, distributedRewards, clubLeaderAddress, nHeight, false);

    return ret;
}

bool CRwdBalanceViewDB::EntrustByAddress(string inputAddr, string voutAddress, int nHeight, bool isUndo)
{
    if (isUndo)
    {
        if (GetFather(voutAddress, nHeight).compare("0") == 0)
        {
            if (!DeleteDB(inputAddr, nHeight+1))
                return false;
            //UpdateMemberDB(voutAddress, isAdd, inputAddr, isUndo);
            //UpdateMemberDB(inputAddr, isDel, voutAddress, isUndo);
        }
        if (!DeleteDB(voutAddress, nHeight+1))
            return false;
        return true;
    }

    // Address of vout must have father of "0" in database or entrust itself
    if ((voutAddress.compare(inputAddr) != 0) && (GetFather(voutAddress, nHeight-1).compare("0") == 0))
    {
        // Input address update
        UpdateCacheFather(inputAddr, nHeight-1, voutAddress);

        //UpdateMemberDB(voutAddress, isAdd, inputAddr, isUndo);
        //UpdateMemberDB(inputAddr, isDel, voutAddress, isUndo);
    }
    else if((voutAddress.compare(inputAddr) == 0) && (GetFather(voutAddress, nHeight-1).compare("0") != 0))
    {
        // Input address update
        UpdateCacheFather(inputAddr, nHeight-1, "0");

        //UpdateMemberDB(voutAddress, isAdd, inputAddr, isUndo);
        //UpdateMemberDB(inputAddr, isDel, voutAddress, isUndo);
    }

    // Address of vout update by transaction count
    UpdateCacheTcAddOne(voutAddress, nHeight-1);

    return true;
}

bool CRwdBalanceViewDB::TcAddOneByAddress(string address, int nHeight, string father, bool isUndo)
{
    if (isUndo)
    {
        if (!DeleteDB(address, nHeight+1))
            return false;
        return true;
    }

    CAmount rewardbalance = 0;
    string ft = " ";
    uint64_t tc = 1;
    GetFullRecord(address, nHeight-1, ft, tc, rewardbalance);
    if (ft.compare(" ") == 0)
    {
        ft = father;
        //UpdateMemberDB(voutAddress, isAdd, inputAddr, isUndo);
    }
    tc++;

    string newRecord = GenerateRecord(ft, tc, rewardbalance);
    cacheRecord[address] = newRecord;

    //if (!WriteDB(address, nHeight, ft, tc, rewardbalance))
    //    return false;
    //LogPrintf("%s, member:%s, rw:%d\n", __func__, member, rewardbalance_old);

    return true;
}

bool CRwdBalanceViewDB::UpdateFatherTCByTX(const CTransaction& tx, const CCoinsViewCache& view, int nHeight, bool isUndo)
{
    if (!tx.IsCoinBase())
    {
        // get best father
        string bestFather = " ";
        if (!isUndo)
        {
            CAmount maxValue = 0;
            for(unsigned int j = 0; j < tx.vin.size(); j++)
            {
                CTxOut out = view.GetOutputFor(tx.vin[j]);
                CAmount val = out.nValue;
                if (val > maxValue)
                {
                    maxValue = val;

                    const CScript script = out.scriptPubKey;
                    CBitcoinAddress addr;
                    if (!addr.ScriptPub2Addr(script, bestFather))
                        return false;
                }
            }
            for(unsigned int k = 0; k < tx.vreward.size(); k++)
            {
                CAmount val = tx.vreward[k].rewardBalance;
                if (val > maxValue)
                {
                    maxValue = val;

                    const CScript script = CScript() << ParseHex(tx.vreward[k].senderPubkey) << OP_CHECKSIG;
                    CBitcoinAddress addr;
                    if (!addr.ScriptPub2Addr(script, bestFather))
                        return false;
                }
            }
        }

        // update packer, father and tc
        for(unsigned int i = 0; i < tx.vout.size(); i++)
        {
            CTxDestination dst;
            ExtractDestinationFromP2PKAndP2PKH(tx.vout[i].scriptPubKey, dst);
            string voutAddress = CBitcoinAddress(dst).ToString();

            if (tx.vout[i].nValue == 0)
            {
                if (!EntrustByAddress(bestFather, voutAddress, nHeight, isUndo))
                    return false;
            }
            else
            {
                if (!TcAddOneByAddress(voutAddress, nHeight, bestFather, isUndo))
                    return false;
            }
        }
    }

    return true;
}

bool CRewardRateViewDB::WriteDB(int nHeight, std::string address, double value)
{
    std::stringstream ssVal;
    ssVal << value;
    std::string strValue;
    ssVal >> strValue;

    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Put(leveldb::WriteOptions(), strHeight, address+"_"+strValue);
    if(!status.ok())
    {
        LogPrintf("LevelDB write failure in reward rate module: %s\n", status.ToString());
        dbwrapper_private::HandleError(status);
        return false;
    }

    return true;
}

bool CRewardRateViewDB::ReadDB(int nHeight, std::string& address_value)
{
    std::stringstream ssHeight;
    std::string strHeight;
    ssHeight << nHeight;
    ssHeight >> strHeight;

    leveldb::Status status = pdb->Get(leveldb::ReadOptions(), strHeight, &address_value);
    if(!status.ok())
    {
        LogPrintf("LevelDB read failure in reward rate module: %s\n", status.ToString());
        dbwrapper_private::HandleError(status);
        return false;
    }

    return true;
}

CRewardRateViewDB::CRewardRateViewDB()
{
    options.create_if_missing = true;

    std::string db_path = GetDataDir(true).string() + std::string("/rewardrate");
    LogPrintf("Opening LevelDB in %s\n", db_path);

    leveldb::Status status = leveldb::DB::Open(options, db_path, &pdb);
    dbwrapper_private::HandleError(status);
    assert(status.ok());
    LogPrintf("Opened LevelDB successfully\n");
}

CRewardRateViewDB::~CRewardRateViewDB()
{
    delete pdb;
    pdb = NULL;
}

bool CRewardRateViewDB::GetRewardRate(int nHeight, string& addr_rate)
{
    if (!ReadDB(nHeight, addr_rate))
        return false;

    return true;
}

bool CRewardRateViewDB::UpdateRewardRate(std::string leaderAddress, double val, int nHeight)
{
    if ((val < 0 || val > 1.0) && val != -1)
        return false;
    if (!WriteDB(nHeight, leaderAddress, val))
        return false;

    return true;
}
