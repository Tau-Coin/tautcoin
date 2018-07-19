#include <iostream>
#include <vector>
#include <cstdlib>

#include "cmdline.h"
#include "isnreward.h"

using namespace std;

/*
 connect isn database
 0 - successful
 */
class ISNDB
{
public:
	ISNDB();

	// select from ISNDB according to address
	mysqlpp::StoreQueryResult ISNSqlSelectAA(string tablename, vector<string> field, string condition, string cvalue);

	// update ISNDB with condition
	mysqlpp::SimpleResult ISNSqlUpdate(string tablename, vector<string> field, vector<string> values, string condition);
	mysqlpp::SimpleResult ISNSqlAddOne(string tablename, vector<string> field, string condition);

	// insert ISNDB with condition
	int ISNSqlInsert(string tablename, vector<string> values);

private:
	mysqlpp::Connection con;
};
