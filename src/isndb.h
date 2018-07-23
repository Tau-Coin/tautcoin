// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ISNCOIN_DATABASE_H
#define ISNCOIN_DATABASE_H

#include <iostream>
#include <vector>
#include <cstdlib>

#include <cmdline.h>
#include <mysql++.h>
#include <ssqls.h>
#include "amount.h"

using namespace std;

//Database info
const char DBName[32]= "imreward";
const char hostName[32]= "192.168.2.215";
const char userName[32]= "immysql";
const char passWord[32]= "im123456";

//Table info
const string tableClub= "clubInfo";
const string clubFieldID= "club_id";
const string clubFieldAddress= "address";
const string clubFieldCount= "ttc";

const string tableMember= "memberInfo";
const string memFieldID= "address_id";
const string memFieldAddress= "address";
const string memFieldClub= "club_id";
const string memFieldFather= "father";
const string memFieldCount= "tc";
const string memFieldBalance= "balance";

/*
 connect isn database
 0 - successful
 */
class ISNDB
{
public:
	ISNDB();

	// select from ISNDB according to address
	mysqlpp::StoreQueryResult ISNSqlSelectAA(const string &tablename, const vector<string> &field, const string &condition, const string &cvalue);

	// update ISNDB with condition
	mysqlpp::SimpleResult ISNSqlUpdate(const string &tablename, const vector<string> &field, const vector<string> &values, const string &condition);
	mysqlpp::SimpleResult ISNSqlAddOne(const string &tablename, const vector<string> &field, const string &condition);

	// insert ISNDB with condition
	int ISNSqlInsert(const string &tablename, const vector<string> &values);

private:
	mysqlpp::Connection con;
};

CAmount getBalanceByAddress(const string& address);
#endif // ISNCOIN_DATABASE_H
