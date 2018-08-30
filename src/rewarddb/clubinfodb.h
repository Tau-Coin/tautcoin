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

#include "clubleaderdb.h"

/** View on the reward rate dataset. */
#define RWDBALDBRATEPATH "/rewardrate"
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

/** View on the club info dataset. */
#define DBSEPECTATOR "_"
#define CLUBINFOPATH "/clubinfo"
class CClubInfoDB
{
private:
    //! the database itself
    leveldb::DB* pdb;

    //! database options used
    leveldb::Options options;

    //! clubinfo database used
    CRewardRateViewDB* _prewardratedbview;

    //!club leader database
    CClubLeaderDB* pclubleaderdb;

    //! cache for multi-transaction balance updating
    std::map<std::string, std::string> cacheRecord;

    //! cache for accelerating
    std::map<std::string, std::vector<std::string> > cacheForRead;

    bool WriteDB(std::string key, int nHeight, std::string strValue);

    bool ReadDB(std::string key, int nHeight, std::string& strValue);

    bool DeleteDB(std::string key, int nHeight);

public:
    //! Constructor
    CClubInfoDB();
    CClubInfoDB(CRewardRateViewDB* prewardratedbview);

    //! Destructor
    ~CClubInfoDB();

    //! Retrieve the reward rate dataset pointer
    CRewardRateViewDB* GetRewardRateDBPointer() const;

    //! Clear the clubinfo cache
    void ClearCache();

    //! Clear the clubinfo accelerating cache
    void ClearReadCache();

    //! Commit the database transaction
    bool Commit(int nHeight);

    //! Update the leader's members
    bool UpdateMembersByFatherAddress(std::string fatherAddress, bool add, std::string address,
                                      int nHeight, bool isUndo);

    //! Retrieve the merbers' addresses in type of trie
    std::string GetTrieStrByFatherAddress(std::string fatherAddress, int nHeight);

    //! Retrieve the merbers' addresses
    std::vector<std::string> GetTotalMembersByAddress(std::string fatherAddress, int nHeight);

    //! Add club leader
    bool AddClubLeader(std::string address);

    //! Remove club leader
    bool RemoveClubLeader(std::string address);

    //! Retrieve all the club leaders
    bool GetAllClubLeaders(std::vector<std::string>& leaders);
};

#endif // TAUCOIN_CLUBINFODB_H
