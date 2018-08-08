// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "isndb.h"

#include "util.h"
#include "sync.h"
#include "util.h"

#include<exception>
#include<fstream>
#include<stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem.hpp>

static CCriticalSection cs_isndb;

ISNDB* ISNDB::pIsnDBSingleton = NULL;

string ISNDB::mysqldbname = "";
string ISNDB::mysqlserver = "";
string ISNDB::mysqlusername = "";
string ISNDB::mysqlpassword = "";

string ISNDB::CREATE_CLUB_TABLE = string("create table clubinfo(")
            + string("club_id INT NOT NULL AUTO_INCREMENT, ")
            + string("address VARCHAR(128) NOT NULL, ")
            + string("ttc BIGINT NOT NULL, ")
            + string("PRIMARY KEY(club_id))")
            + string("ENGINE=InnoDB DEFAULT CHARSET=utf8;");

string ISNDB::CREATE_MEMBER_TABLE = string("create table memberinfo(")
            + string("address_id INT NOT NULL AUTO_INCREMENT, ")
            + string("address VARCHAR(128) NOT NULL, ")
            + string("club_id INT NOT NULL, ")
            + string("father INT NOT NULL, ")
            + string("tc BIGINT NOT NULL, ")
            + string("balance BIGINT NOT NULL, ")
            + string("PRIMARY KEY(address_id))")
            + string("ENGINE=InnoDB DEFAULT CHARSET=utf8;");


bool ISNDB::BootupPreCheck()
{
    boost::filesystem::path cfgFile = GetDataDir() / "mysql.cfg";
    if (!boost::filesystem::exists(cfgFile) && (!mapArgs.count("-mysqlusername")
        || !mapArgs.count("-mysqlpassword") || GetArg("-mysqlusername", "").empty()))
    {
        return false;
    }

    return true;
}

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

void ISNDB::LoadMysqlCfg()
{
    boost::filesystem::path cfgFile = GetDataDir() / "mysql.cfg";
    if (!boost::filesystem::exists(cfgFile))
        return;

    const int LINE_LENGTH = 100;
    char str[LINE_LENGTH] = {0};
    ifstream f(cfgFile.string());
    if (!f)
    {
        LogPrintf("%s warning: open cfg file failed\n", __func__);
        return;
    }

    vector<string> vStrInputParts;
    while(f.getline(str, LINE_LENGTH))
    {
        string line(str);
        vStrInputParts.clear();
        boost::split(vStrInputParts, line, boost::is_any_of(":"));
        if (vStrInputParts.size() == 0)
            continue;

        if (vStrInputParts[0] == "mysqldbname")
        {
            mysqldbname = vStrInputParts[1];
        }
        else if (vStrInputParts[0] == "mysqlserver")
        {
            mysqlserver = vStrInputParts[1];
        }
        else if (vStrInputParts[0] == "mysqlusername")
        {
            mysqlusername = vStrInputParts[1];
        }
        else if (vStrInputParts[0] == "mysqlpassword")
        {
            mysqlpassword = vStrInputParts[1];
        }
    }
    f.close();

    LogPrintf("%s %s %s %s %s\n", __func__, mysqldbname, mysqlserver, mysqlusername, mysqlpassword);
}

void ISNDB::StoreMysqlCfg()
{
    boost::filesystem::path cfgFile = GetDataDir() / "mysql.cfg";
    ofstream ofs(cfgFile.string());
    if (!ofs)
    {
        LogPrintf("%s warning: open cfg file failed\n", __func__);
        return;
    }

    ofs << "mysqldbname:"   << mysqldbname << endl;
    ofs << "mysqlserver:"   << mysqlserver << endl;
    ofs << "mysqlusername:" << mysqlusername << endl;
    ofs << "mysqlpassword:" << mysqlpassword << endl;

    ofs.close();
}

void ISNDB::InitMysqlCfg()
{
    // First read configurations where cfg file "mysql.cfg".
    LoadMysqlCfg();

    // Use specified value from arguments
    if (mapArgs.count("-mysqldbname"))
    {
        mysqldbname = GetArg("-mysqldbname", mysqldbname);
    }
    if (mapArgs.count("-mysqlserver"))
    {
        mysqlserver = GetArg("-mysqlserver", mysqlserver);
    }
    if (mapArgs.count("-mysqlusername"))
    {
        mysqlusername = GetArg("-mysqlusername", mysqlusername);
    }
    if (mapArgs.count("-mysqlpassword"))
    {
        mysqlpassword = GetArg("-mysqlpassword", mysqlpassword);
    }
}

ISNDB::ISNDB()
{
    try {
        InitMysqlCfg();
        if (mysqlusername.empty())
        {
            throw std::logic_error("mysql username is empty");
        }
        if (mysqlpassword.empty())
        {
            LogPrintf("isndb warning: mysql password is empty.\n");
        }
        if (mysqldbname.empty())
        {
            mysqldbname = DBName;
        }
        if (mysqlserver.empty())
        {
            mysqlserver = hostName;
        }
        StoreMysqlCfg();

        LogPrintf("Mysql cfg: username:%s, password:%s, db:%s, server:%s\n",
            mysqlusername, mysqlpassword, mysqldbname, mysqlserver);

        LOCK(cs_isndb);
        con = mysqlpp::Connection(NULL, mysqlserver.c_str(), mysqlusername.c_str(), mysqlpassword.c_str());
        poption = new mysqlpp::ReconnectOption(true);
        // Objects passed to "Objects passed to this method and successfully set will be released
        // when this Connection object is destroyed.
        // So there is no need to obviously destroy "poption".
        con.set_option(poption);

        try
        {
            con.select_db(mysqldbname);
            mysqlpp::Query query = con.query();
            // If blocks directory don't exist, truncate tables;
            boost::filesystem::path blocksDir = GetDataDir() / "blocks";
            if (!boost::filesystem::exists(blocksDir))
            {
                LogPrintf("truncate tables...\n");
                query.exec("truncate table clubinfo");
                query.exec("truncate table memberinfo");
            }
        }
        catch (const mysqlpp::DBSelectionFailed& er)
        {
            LogPrintf("DBSelectionFailed: %d, %s\n", er.errnum(), er.what());
            // err code 1049 means database not exist
            // Create database and tables;
            string errstr(er.what());
            if (er.errnum() == 1049 && (errstr.find("Unknown database") != string::npos))
            {
                if (con.create_db(mysqldbname) && con.select_db(mysqldbname))
                {
                    LogPrintf("create tables...\n");
                    mysqlpp::Query query = con.query();
                    query << CREATE_CLUB_TABLE;
                    query.execute();
                    query << CREATE_MEMBER_TABLE;
                    query.execute();
                }
                else
                {
                    throw std::runtime_error("create or select database failed");
                }
            }
        }
	}
	catch (const mysqlpp::Exception& er) {
		// Catch-all for any other MySQL++ exceptions
		cerr << "Error: " << er.what() << endl;
		exit(-1);
	}
	catch (const std::exception& er) {
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
        LOCK(cs_isndb);
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
        LOCK(cs_isndb);
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
        LOCK(cs_isndb);
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
        LOCK(cs_isndb);
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
        LOCK(cs_isndb);
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
