#ifndef TAUCOIN_CLUBINFODB_H
#define TAUCOIN_CLUBINFODB_H

#include "coins.h"
#include "coinsbyscript.h"
#include "dbwrapper.h"
#include "chain.h"
#include "base58.h"
#include "leveldb/db.h"
#include <map>
#include <string>
#include <vector>


#define CLUBINFODBPATH "clubinfodb"
#define RWDBALDBRATEPATH "/rewardrate"
#define NOT_VALID_RECORD "NOT_VALID"
#define NO_MOVED_ADDRESS "NO_MOVED_ADDRESS"

extern CCriticalSection cs_clubinfo;

/** View on the reward rate dataset. */

class CRewardRateViewDB
{
private:
    //! the database itself
    leveldb::DB* pdb;

    //! database options used
    leveldb::Options options;

    bool WriteDB(int nHeight, std::string address, double value);

    bool ReadDB(int nHeight, std::string& address_value);

public:
    //! Constructor
    CRewardRateViewDB();

    //! Destructor
    ~CRewardRateViewDB();

    //! Retrieve the reward rate for a given address
    bool GetRewardRate(int nHeight, std::string& addr_rate);

    //! Update the reward rate dataset represented by view
    bool UpdateRewardRate(std::string leaderAddress, double val, int nHeight);
};

typedef struct _CMemberInfo {
    std::string address; // The address of this info

    uint64_t MP; // The total mining power of the address, when the address is a miner

    CAmount rwd; // The reward balance of the address

    _CMemberInfo() : address(" "), MP(0), rwd(0) { }

    _CMemberInfo(std::string _address, uint64_t _MP, CAmount _rwd) :
        address(_address), MP(_MP), rwd(_rwd) { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(address);
        READWRITE(MP);
        READWRITE(rwd);
    }

}CMemberInfo;

/** View on the club info dataset. */
class CClubInfoDB : public CDBWrapper
{
private:
    //! the level DB's WriteBatch
    CDBBatch* batch;

    //! clubinfo database used
    CRewardRateViewDB* _prewardratedbview;

    //! cache for accelerating
    std::map<std::string, std::vector<CMemberInfo> > cacheRecord;

    //! Current updated height
    int currentHeight;

    bool WriteDB(const std::string& address, const std::vector<CMemberInfo>& value);
    template <typename K, typename V>
    void WriteToBatch(const K& key, const V& value) { batch->Write(key, value); }

    bool ReadDB(const std::string& address, std::vector<CMemberInfo>& value) const;

    bool DeleteDB(const std::string& address);
    void DeleteToBatch(const std::string& address) { batch->Erase(address); }

    bool CommitDB(bool fSync = false) { return WriteBatch(*batch, fSync); }

    void GetTotalMembers(const std::string& fatherAddress, std::vector<std::string>& vmembers);

    bool ComputeMemberReward(const uint64_t& MP, const uint64_t& totalMP,
                             const CAmount& totalRewards, CAmount& memberReward);

    bool UpdateRewards(const std::string& minerAddress, CAmount memberRewards,
                       uint64_t memberTotalMP, CAmount& distributedRewards, bool isUndo);

public:
    //! Constructor
    CClubInfoDB(size_t nCacheSize, bool fMemory=false, bool fWipe=false);
    CClubInfoDB(size_t nCacheSize, CRewardRateViewDB* prewardratedbview, bool fMemory=false, bool fWipe=false);

    //! Destructor
    ~CClubInfoDB();

    //! Retrieve the reward rate dataset pointer
    CRewardRateViewDB* GetRewardRateDBPointer() const;

    //! Check if input address is valid
    static bool AddressIsValid(std::string address);

    //! Init the father and mp of the address from genesis block
    bool InitGenesisDB(const std::vector<std::string>& addresses);

    //! Clear the clubinfo accelerating cache
    void ClearCache();

    //! Commit the database transaction
    bool Commit();

    //! Set current updated height
    void SetCurrentHeight(int nHeight);

    //! Get current updated height
    int GetCurrentHeight() const;

    //! Read data from disk to memory
    bool LoadDBToMemory();

    //! Write data from memory to disk
    bool WriteDataToDisk(int newestHeight, bool fSync);

    //! Retrieve the existence of the address's item
    bool CacheRecordIsExist(const std::string& address);

    //! Get cache record by address
    std::vector<CMemberInfo> GetCacheRecord(const std::string& address);

    //! Update the leader's members
    std::string UpdateMembersByFatherAddress(const std::string& fatherAddress, const CMemberInfo& memberinfo,
                                             uint64_t& index, int nHeight, bool add);

    //! Retrieve the merbers' addresses
    void GetTotalMembersByAddress(const std::string& fatherAddress, std::vector<std::string>& vmembers);

    //! Update rewards of all members in the club
    bool UpdateRewardsByMinerAddress(const std::string& minerAddress, CAmount memberRewards,
                                     uint64_t memberTotalMP, CAmount& distributedRewards, bool isUndo);

    //! Update mining power of the address
    bool UpdateMpByChange(std::string fatherAddr, uint64_t index, bool isUndo=false,
                          uint64_t amount=1, bool add=true);

    //! Update reward of the address
    void UpdateRewardByChange(std::string fatherAddr, uint64_t index, CAmount rewardChange, bool isUndo=false);

    //! Get all the fathers
    std::vector<std::string> GetAllFathers();
};

#endif // TAUCOIN_CLUBINFODB_H
