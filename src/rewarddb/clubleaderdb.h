// Copyright (c) 2018- The Taucoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TAUCOIN_CLUBLEADERDB_H
#define TAUCOIN_CLUBLEADERDB_H

#include "coins.h"
#include "coinsbyscript.h"
#include "dbwrapper.h"
#include "chain.h"
#include "base58.h"
#include "leveldb/db.h"
#include <map>
#include <string>
#include <vector>

struct COPComparator
{
    bool operator()(int h1, int h2) const {
        // First sort by height
        if (h1 > h2) return false;
        if (h1 < h2) return true;

        return false;
    }
};

typedef std::map<int, std::string, COPComparator> OpMap;

class CClubLeaderCache
{
public:
    int height;
    std::map<std::string, OpMap> cache;

    CClubLeaderCache()
    {
        height = -1;
        cache.clear();
    }
};

class CClubLeaderDB
{
private:
    //! the database itself
    leveldb::DB* pdb;

    //! database options used
    leveldb::Options options;

    // !database patch
    static const std::string DB_PATH;

    // !database prefix
    static const std::string DB_LEADER;

    static const std::string KEY_SPERATOR;

    //! cache for club leaders updating
    std::map<std::string, std::string> cache;

    //! add operation
    static const std::string ADD_OP;

    //! remove operation
    static const std::string REMOVE_OP;

    CClubLeaderCache* pdbcache;

    bool Write(std::string address, std::string height);

    bool Delete(std::string address, std::string height);

    bool SyncWithDB();

    bool SeletedOrNot(OpMap& operations, int height);

    void DumpDBCache();

public:
    //! Constructor
    CClubLeaderDB();

    //! Destructor
    ~CClubLeaderDB();

    //! Commit the database transaction
    bool Commit();

    //! Add club leader
    bool AddClubLeader(std::string address, int height);

    //! Remove club leader
    bool RemoveClubLeader(std::string address, int height);

    // ! Delete club leader from db
    bool DeleteClubLeader(std::string address, int height);

    //! Retrieve all the club leaders
    // Note: this method is just for debug!
    bool GetAllClubLeaders(std::vector<std::string>& leaders, int height);
};

#endif //TAUCOIN_CLUBLEADERDB_H
