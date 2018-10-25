#ifndef TAUCOIN_ADDRINFODB_H
#define TAUCOIN_ADDRINFODB_H

#include "coins.h"
#include "coinsbyscript.h"
#include "dbwrapper.h"
#include "chain.h"
#include "base58.h"
#include "leveldb/db.h"
#include "clubinfodb.h"
#include <stdio.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#define ADDRINFODBPATH "addrinfodb"
#define NEWESTHEIGHFLAG (-10)

extern CCriticalSection cs_addrinfo;

typedef struct _CTAUAddrInfo {
    std::string miner; // The miner of the address

    std::string father; // The father of the address

    uint64_t index; // The address's index in the children of the father

    uint64_t totalMP; // The total mining power of the address, when the address is a miner

    int lastHeight; // The height when last record update

    _CTAUAddrInfo() : miner(" "), father(" "), index(0), totalMP(0), lastHeight(-1) { }

    _CTAUAddrInfo(std::string _miner, std::string _father, uint64_t _index, uint64_t _totalMP,
                  int _lastHeight=-1) :
        miner(_miner), father(_father), index(_index), totalMP(_totalMP),
        lastHeight(_lastHeight) { }

    _CTAUAddrInfo& operator=(const _CTAUAddrInfo& b)
    {
        miner = b.miner;
        father = b.father;
        index = b.index;
        totalMP = b.totalMP;
        lastHeight = b.lastHeight;
        return *this;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(miner);
        READWRITE(father);
        READWRITE(index);
        READWRITE(totalMP);
        READWRITE(lastHeight);
    }

}CTAUAddrInfo;

/** View on the address info dataset. */
class CAddrInfoDB : public CDBWrapper
{
private:
    //! the level DB's WriteBatch
    CDBBatch* batch;

    //! cache for db's multi-transaction updating
    std::map<std::string, CTAUAddrInfo> cacheRecord;

    //! cache for accelerating
    std::map<std::string, CTAUAddrInfo> cacheForRead;

    //! cache for add members to the address
    std::map<std::string, std::string> cacheForClubAdd;

    //! cache for delete members from the address
    std::map<std::string, std::string> cacheForClubRm;

    //! cache for erase cacheRecord
    std::set<std::string> cacheForErs;

    //! cache used for undo
    std::map<std::string, CTAUAddrInfo> cacheForUndo;

    //! cache used for undo acceleration
    std::map<std::string, CTAUAddrInfo> cacheForUndoRead;

    //! cache used for height of undo in db
    std::map<std::string, int> cacheForUndoHeight;

    //! clubinfo database used
    CClubInfoDB* _pclubinfodb;

    //! Current updated height
    int currentHeight;

    void WriteToBatch(const std::string& address, int nHeight, const CTAUAddrInfo& value);
    void WriteNewestToBatch(const std::string& address, const CTAUAddrInfo& value);
    bool WriteDB(const std::string& address, int nHeight, const CTAUAddrInfo& value);
    template <typename K, typename V>
    void WriteToBatch(const K& key, const V& value) { batch->Write(key, value); }

    bool ReadDB(const std::string& address, int nHeight, CTAUAddrInfo& value) const;

    void DeleteToBatch(const std::string& address, int nHeight);
    bool DeleteDB(const std::string& address, int nHeight);
    template <typename K>
    void DeleteToBatch(const K& key) { batch->Erase(key); }

    bool CommitDB(bool fSync = false) { return WriteBatch(*batch, fSync); }

    bool RewardChangeUpdateByPubkey(CAmount rewardChange, std::string pubKey, int nHeight, bool isUndo=false);
    bool RewardChangeUpdate(CAmount rewardChange, std::string address, int nHeight, bool isUndo=false);

    void UpdateMembersByFatherAddress(const std::string& fatherAddress, const CMemberInfo& memberinfo,
                                      uint64_t& addrIndex, int nHeight, bool add, bool isUndo=false);

    bool GetBestFather(const CTransaction& tx, const CCoinsViewCache &view, std::string& bestFather,
                       std::map<std::string, CAmount> vin_val=std::map<std::string, CAmount>(), bool isUndo=false);

    bool UpdateCacheRecord(std::string address, int inputHeight, std::string newFather,
                           std::string newMiner, uint64_t newIdx);

    bool UpdateCacheMpAddOne(std::string address, int nHeight, bool isUndo=false);

    bool UpdateCacheTotalMPByChange(std::string address, int nHeight, uint64_t amount, bool isAdd);

    bool UpdateCacheRewardChange(std::string address, int inputHeight, CAmount rewardChange, bool isUndo=false);

public: 
    //! Constructor
    CAddrInfoDB(size_t nCacheSize, CClubInfoDB *pclubinfodb, bool fMemory=false, bool fWipe=false);

    //! Destructor
    ~CAddrInfoDB();

    //! Clear the rwdbalance cache
    void ClearCache();

    //! Clear the rwdbalance cache
    void ClearUndoCache();

    //! Clear the rwdbalance accelerating cache
    void ClearReadCache();

    //! Commit the database transaction
    bool Commit(int nHeight, bool isUndo=false);

    //! Set current updated height
    void SetCurrentHeight(int nHeight);

    //! Get current updated height
    int GetCurrentHeight() const;

    //! Read the newest data from disk to memory
    bool LoadNewestDBToMemory();

    //! Write the newest data from memory to disk
    bool WriteNewestDataToDisk(int newestHeight, bool fSync=false);

    //! Init the father and mp of the address from genesis block
    bool InitGenesisDB(const std::vector<std::string>& addresses);

    //! Retrieve the miner for a given address
    std::string GetMiner(std::string address, int nHeight);

    //! Retrieve the father for a given address
    std::string GetFather(std::string address, int nHeight);

    //! Retrieve the mining power for a given address
    bool GetMiningPower(std::string address, int nHeight, uint64_t& miningPower);

    //! Retrieve the total mining power of the whole club where a given address in
    uint64_t GetTotalMP(std::string address, int nHeight);

    //! Retrieve the reward balance for a given address
    CAmount GetRwdBalance(std::string address, int nHeight);

    //! Retrieve a full record for a given address
    CTAUAddrInfo GetAddrInfo(std::string address, int nHeight);

    //! Retrieve the harvest power for a given address if it's a miner
    uint64_t GetHarvestPowerByAddress(std::string address, int nHeight);

    //! Someone entrust another
    bool EntrustByAddress(std::string inputAddr, std::string voutAddress, int nHeight);

    //! A common TX that update the mining power and totalMP
    bool UpdateMpAndTotalMPByAddress(std::string address, int nHeight, std::string fatherInput);

    //! Update the Balance dataset
    bool UpdateRewardsByTX(const CTransaction& tx, CAmount blockReward, int nHeight, bool isUndo=false);

    //! Update the mining power and the father
    bool UpdateFatherAndMpByTX(const CTransaction& tx, const CCoinsViewCache &view, int nHeight,
                               std::map<std::string, CAmount> vin_val=std::map<std::string, CAmount>());

    //! Undo the mining power and init relationship undo
    bool UndoMiningPowerByTX(const CTransaction& tx, const CCoinsViewCache& view, int nHeight,
                             std::map<std::string, CAmount> vin_val);

    //! Undo clubInfo cache
    bool UndoClubMembers(int nHeight);

    //! Undo cache records
    void UndoCacheRecords(int nHeight);

    //! Update the club miner's distribution rate
    bool RewardRateUpdate(CAmount blockReward, CAmount distributedRewards, std::string clubLeaderAddress, int nHeight);

    //! Get all the club miners
    std::vector<std::string> GetAllClubMiners();
};

#endif // TAUCOIN_ADDRINFODB_H
