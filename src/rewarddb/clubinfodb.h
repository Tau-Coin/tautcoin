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

    //! cache for multi-transaction balance updating
    std::map<std::string, std::string> cacheRecord;

    bool WriteDB(std::string key, int nHeight, std::string strValue);

    bool ReadDB(std::string key, int nHeight, std::string& strValue);

    bool DeleteDB(std::string key, int nHeight);

public:
    //! Constructor
    CClubInfoDB();

    //! Destructor
    ~CClubInfoDB();

    //! Clear the rwdbalance cache
    void ClearCache();

    //! Commit the database transaction
    bool Commit(int nHeight);

    //! Update the leader's members
    bool UpdateMembersByFatherAddress(std::string fatherAddress, bool add, std::string address,
                                      int nHeight, bool isUndo);

    //! Retrieve the merbers' addresses in type of trie
    std::string GetTrieStrByFatherAddress(std::string fatherAddress, int nHeight);

    //! Retrieve the merbers' addresses
    std::vector<std::string> GetTotalMembersByAddress(std::string fatherAddress, int nHeight);
};

#endif // TAUCOIN_CLUBINFODB_H
