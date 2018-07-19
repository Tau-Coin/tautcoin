/***********************************************************************
 dbinfo.cpp - Example showing how to request information about the
	database schema, such as table names, column types, etc.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB, and
 (c) 2004-2009 by Educational Technology Resources, Inc.  Others may
 also hold copyrights on code in this file.  See the CREDITS.txt file
 in the top directory of the distribution for details.

 This file is part of MySQL++.

 MySQL++ is free software; you can redistribute it and/or modify it
 under the terms of the GNU Lesser General Public License as published
 by the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.

 MySQL++ is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with MySQL++; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 USA
***********************************************************************/

#include "cmdline.h"
#include "printdata.h"

#include <mysql++.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;


// Insert a bar into the stream with the given query string centered
static void
separator(ostream& os, string qstr)
{
	string sep("========================================"
			"========================================");
	if (qstr.size()) {
		string::size_type start = (sep.size() - qstr.size()) / 2;
		sep.replace(start - 1, 1, 1, ' ');
		sep.replace(start, qstr.size(), qstr);
		sep.replace(start + qstr.size(), 1, 1, ' ');
		os << "\n\n";
	}
	os << sep << endl;
}


// Print out the MySQL server version
static void
show_mysql_version(mysqlpp::Connection& con)
{
	separator(cout, "");
    cout << "MySQL version: " << con.client_version();
}


// Print out the names of all the databases managed by the server
static void
show_databases(mysqlpp::Connection& con)
{
	mysqlpp::Query query = con.query("show databases");
	separator(cout, query.str());
	mysqlpp::StoreQueryResult res = query.store();

	cout << "Databases found: " << res.size();
	cout.setf(ios::left);
	mysqlpp::StoreQueryResult::iterator rit;
	for (rit = res.begin(); rit != res.end(); ++rit) {
		cout << "\n\t" << (*rit)[0];
	}
}


// Print information about each of the tables we found
static void
show_table_info(mysqlpp::Connection& con, const vector<string>& tables)
{
	vector<string>::const_iterator it;
	for (it = tables.begin(); it != tables.end(); ++it) {
		mysqlpp::Query query = con.query();
		query << "describe " << *it;
		separator(cout, query.str());
		mysqlpp::StoreQueryResult res = query.store();

		size_t columns = res.num_fields();
		vector<size_t> widths;
		for (size_t i = 0; i < columns; ++i) {
			string s = res.field_name(int(i));
			if (s.compare("field") == 0) {
				widths.push_back(22);
			}
			else if (s.compare("type") == 0) {
				widths.push_back(20);
			}
			else if (s.compare("null") == 0) {
				widths.push_back(4);
			}
			else if (s.compare("key") == 0) {
				widths.push_back(3);
			}
			else if (s.compare("extra") == 0) {
				widths.push_back(0);
			}
			else {
				widths.push_back(15);
			}

			if (widths[i]) {
				cout << '|' << setw(widths[i]) << 
						res.field_name(int(i)) << '|';
			}
		}
		cout << endl;

		mysqlpp::StoreQueryResult::iterator rit;
		for (rit = res.begin(); rit != res.end(); ++rit) {
			for (unsigned int i = 0; i < columns; ++i) {
				if (widths[i]) {
					cout << ' ' << setw(widths[i]) <<
							(*rit)[i].c_str() << ' ';
				}
			}
			cout << endl;
		}
	}
}


// Print out the names of all tables in the sample database, and
// return the list of tables.
static void
show_tables(mysqlpp::Connection& con)
{
	mysqlpp::Query query = con.query("show tables");
	separator(cout, query.str());
	mysqlpp::StoreQueryResult res = query.store();

	cout << "Tables found: " << res.size();
	cout.setf(ios::left);
	vector<string> tables;
	mysqlpp::StoreQueryResult::iterator rit;
	for (rit = res.begin(); rit != res.end(); ++rit) {
		string tbl((*rit)[0]);
		cout << "\n\t" << tbl;
		tables.push_back(tbl);
	}

	show_table_info(con, tables);
}


// Call all the above functions in sequence
int
main(int argc, char* argv[])
{
	// Get database access parameters from command line
	mysqlpp::examples::CommandLine cmdline(argc, argv);
	if (!cmdline) {
		return 1;
	}

	try {
		// Connect to server, then dump a bunch of stuff we find on it
		mysqlpp::Connection con(mysqlpp::examples::db_name,
				cmdline.server(), cmdline.user(), cmdline.pass());
		show_mysql_version(con);
		show_databases(con);
		show_tables(con);
	}
	catch (const mysqlpp::BadQuery& er) {
		// Handle any query errors
		cerr << "Query error: " << er.what() << endl;
		return -1;
	}
	catch (const mysqlpp::Exception& er) {
		// Catch-all for any other MySQL++ exceptions
		cerr << "Error: " << er.what() << endl;
		return -1;
	}

	return 0;
}
