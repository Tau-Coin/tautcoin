// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "isndb.h"

#include "sync.h"
#include "util.h"

static CCriticalSection cs_isndb;

ISNDB* ISNDB::pIsnDBSingleton = NULL;

void ISNDB::StartISNDBService()
{
    // Start db connection
    LogPrintf("Starting ISNDB service...\n");
    ISNDB::GetInstance();
}

void ISNDB::StopISNDBService()
{
    LogPrintf("Stopping ISNDB service...\n");
    delete(pIsnDBSingleton);
}

ISNDB* ISNDB::GetInstance()
{
    LOCK(cs_isndb);

    if (pIsnDBSingleton == NULL)
    {
        pIsnDBSingleton = new ISNDB();
    }

    return pIsnDBSingleton;
}

ISNDB::ISNDB()
{
	try{
		con= mysqlpp::Connection(DBName, hostName, userName, passWord);
	}
	catch (const mysqlpp::Exception& er) {
		// Catch-all for any other MySQL++ exceptions
		cerr << "Error: " << er.what() << endl;
		exit(-1);
	}
}

ISNDB::~ISNDB()
{
    // Disconnect db connection
    con.disconnect();
}

// select from ISNDB according to address
mysqlpp::StoreQueryResult ISNDB::ISNSqlSelectAA(const string &tablename, const vector<string> &field, const string &condition, const string &cvalue)
{
	// form the query sentence
	int fieldSize= field.size();
    mysqlpp::StoreQueryResult dataTmp;

	try{

		mysqlpp::Query query= con.query();
		//according tablename in different way
		if(fieldSize == 1)
		{
			query<< "select %0"" from %1"" where %2""= %3q";
			query.parse();
            dataTmp = query.store(field[0], tablename, condition, cvalue);
			return dataTmp;
		}
		else if(fieldSize == 2)
		{
            query<< "select %0"", %1"" from %2"" where %3""= %4q";
			query.parse();
            dataTmp = query.store(field[0], field[1], tablename, condition, cvalue);
			return dataTmp;
		}
		else if(fieldSize == 3)
		{
            query<< "select %0"", %1"", %2"" from %3"" where %4"" = %5q";
			query.parse();
            dataTmp = query.store(field[0], field[1], field[2], tablename, condition, cvalue);
			return dataTmp;
		}
	}
	catch (const mysqlpp::BadQuery& er) {
		// Handle any query errors
		cerr << "Query error: " << er.what() << endl;
		exit(-1);
	}
	catch (const mysqlpp::BadConversion& er) {
		// Handle bad conversions; e.g. type mismatch populating 'stock'
		cerr << "Conversion error: " << er.what() << endl <<
				"\tretrieved data size: " << er.retrieved <<
				", actual size: " << er.actual_size << endl;
		exit(-1);
	}
    return dataTmp;
}

// update ISNDB with condition
mysqlpp::SimpleResult ISNDB::ISNSqlUpdate(const string &tablename, const vector<string> &field, const vector<string> &values, const string &condition, const string &cvalue)
{
	// form the query sentence
	if(field.size()!=values.size())
	{
		cout<< "Error in update " << endl;
		exit(-1);
	}

	int fieldSize= field.size();
    mysqlpp::SimpleResult dataTmp;

	try{

		mysqlpp::Query query= con.query();
		//according tablename in different way
		if(fieldSize == 1)
		{
			query<< "update %0"" set %1""= %2q where %3""= %4q";
			query.parse();
			mysqlpp::SimpleResult dataTmp = query.execute(tablename, field[0], values[0], condition, cvalue);
			return dataTmp;
		}
		else if(fieldSize == 2)
		{
			query<< "update %0"" set %1""= %2q, %3""= %4q where %5""= %6q";
			query.parse();
			mysqlpp::SimpleResult dataTmp = query.execute(tablename, field[0], values[0], field[1], values[1], condition, cvalue);
			return dataTmp;
		}
		else if(fieldSize == 3)
		{
			query<< "update %0"" set %1""= %2q, %3""= %4q, %5""= %6q where %7""= %8q";
			query.parse();
			mysqlpp::SimpleResult dataTmp = query.execute(tablename,
														  field[0], values[0],
														  field[1], values[1],
														  field[2], values[2],
														  condition, cvalue);
			return dataTmp;
		}
	}
	catch (const mysqlpp::BadQuery& er) {
		// Handle any query errors
		cerr << "Query error: " << er.what() << endl;
        exit(-1);
	}
	catch (const mysqlpp::BadConversion& er) {
		// Handle bad conversions; e.g. type mismatch populating 'stock'
		cerr << "Conversion error: " << er.what() << endl <<
				"\tretrieved data size: " << er.retrieved <<
				", actual size: " << er.actual_size << endl;
        exit(-1);
	}
    return dataTmp;
}
// for ttc+= 1
mysqlpp::SimpleResult ISNDB::ISNSqlAddOne(const string &tablename, const vector<string> &field, const string &condition, const string &cvalue)
{
	try{

		mysqlpp::Query query= con.query();
		//according tablename in different way
		query<< "update %0"" set %1""=%1""+ 1 where %2""= %3q";
		query.parse();
		mysqlpp::SimpleResult dataTmp = query.execute(tablename, field[0], condition, cvalue);
		return dataTmp;
	}
	catch (const mysqlpp::BadQuery& er) {
		// Handle any query errors
		cerr << "Query error: " << er.what() << endl;
		exit(-1);
	}
	catch (const mysqlpp::BadConversion& er) {
		// Handle bad conversions; e.g. type mismatch populating 'stock'
		cerr << "Conversion error: " << er.what() << endl <<
				"\tretrieved data size: " << er.retrieved <<
				", actual size: " << er.actual_size << endl;
		exit(-1);
	}
}
// insert ISNDB with condition
long ISNDB::ISNSqlInsert(const string &tablename, const vector<string> &values)
{
	try{
        if(tablename==tableClub)
		{
			mysqlpp::Query query= con.query();
			query << "insert clubinfo(address, ttc) values(%0q, %1q)";
			query.parse();
			mysqlpp::SimpleResult dataTmp = query.execute(values[0], values[1]);
			return dataTmp.insert_id();
		}
        else if(tablename==tableMember)
		{
			mysqlpp::Query query= con.query();
			query << "insert memberinfo(address, club_id, father, tc, balance) values(%0q, %1q, %2q, %3q, %4q)";
			query.parse();
			mysqlpp::SimpleResult dataTmp = query.execute(values[0], values[1], values[2], values[3], values[4]);
			return dataTmp.insert_id();
		}
	}
	catch (const mysqlpp::BadQuery& er) {
		// Handle any query errors
		cerr << "Query error: " << er.what() << endl;
		exit(-1);
	}
	catch (const mysqlpp::BadConversion& er) {
		// Handle bad conversions; e.g. type mismatch populating 'stock'
		cerr << "Conversion error: " << er.what() << endl <<
				"\tretrieved data size: " << er.retrieved <<
				", actual size: " << er.actual_size << endl;
		exit(-1);
	}
	
	return 0;
}

// delete from ISNDB
bool ISNDB::ISNSqlDelete(const string &tablename, const string &condition, const string &value)
{
	try{

		mysqlpp::Query query= con.query();
		//according tablename in different way
		query<< "delete from %0"" where %1"" = %2q";
		query.parse();
		mysqlpp::SimpleResult dataTmp = query.execute(tablename, condition, value);
		return true;
	}
	catch (const mysqlpp::BadQuery& er) {
		// Handle any query errors
		cerr << "Query error: " << er.what() << endl;
		exit(-1);
	}
	catch (const mysqlpp::BadConversion& er) {
		// Handle bad conversions; e.g. type mismatch populating 'stock'
		cerr << "Conversion error: " << er.what() << endl <<
				"\tretrieved data size: " << er.retrieved <<
				", actual size: " << er.actual_size << endl;
		exit(-1);
	}
}
