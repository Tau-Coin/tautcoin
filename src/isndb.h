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
	mysqlpp::StoreQueryResult ISNSqlSelectAA(const string &tablename, const vector<string> &field, const string &condition, const string &cvalue);

	// update ISNDB with condition
	mysqlpp::SimpleResult ISNSqlUpdate(const string &tablename, const vector<string> &field, const vector<string> &values, const string &condition);
	mysqlpp::SimpleResult ISNSqlAddOne(const string &tablename, const vector<string> &field, const string &condition);

	// insert ISNDB with condition
	int ISNSqlInsert(const string &tablename, const vector<string> &values);

private:
	mysqlpp::Connection con;
};
