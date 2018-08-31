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

    //! cache for club leaders updating
    std::map<std::string, std::string> cache;

    //! add operation
    static const std::string ADD_OP;

    //! remove operation
    static const std::string REMOVE_OP;

    bool Write(std::string address, std::string height);

    bool Delete(std::string address, std::string height);

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

    //! Retrieve all the club leaders
    bool GetAllClubLeaders(std::vector<std::string>& leaders, int height);
};

#endif //TAUCOIN_CLUBLEADERDB_H
