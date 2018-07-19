/***********************************************************************
 resetdb.cpp - (Re)initializes the example database, mysql_cpp_data.
	You must run this at least once before running most of the other
	examples, and it is helpful sometimes to run it again, as some of
	the examples modify the table in this database.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB, and
 (c) 2004-2009 by Educational Technology Resources, Inc.  Others may
 also hold copyrights on code in this file.  See the CREDITS file in
 the top directory of the distribution for details.

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

#include <iostream>
#include <cstdio>

using namespace std;


// Pull in the sample database name from the cmdline module.
extern const char* kpcSampleDatabase;


// Convert a packed version number in the format used within MySQL++
// to a printable string.
static string
version_str(int packed)
{
	char buf[9];
	snprintf(buf, sizeof(buf), "%d.%d.%d",
			(packed & 0xFF0000) >> 16,
			(packed & 0x00FF00) >> 8,
			(packed & 0x0000FF));
	return buf;
}


int
main(int argc, char *argv[])
{
	// Ensure that we're not mixing library and header file versions.
	// This is really easy to do if you have MySQL++ on your system and
	// are trying to build a new version, and run the examples directly
	// instead of through exrun.
	if (mysqlpp::get_library_version() != MYSQLPP_HEADER_VERSION) {
		cerr << "Version mismatch: library is v" <<
				version_str(mysqlpp::get_library_version()) <<
				", headers are v" <<
				version_str(MYSQLPP_HEADER_VERSION) <<
				".  Are you running this" << endl <<
				"with exrun?  See README.examples." << endl;
		return 1;
	}
	
	// Get connection parameters from command line
	mysqlpp::examples::CommandLine cmdline(argc, argv);
	if (!cmdline) {
		return 1;
	}

	// Connect to database server
	mysqlpp::Connection con;
	try {
		if (cmdline.dtest_mode()) {
			cout << "Connecting to database server..." << endl;
		}
		else {
			const char* u = cmdline.user() ? cmdline.user() : "";
			const char* s = cmdline.server() ? cmdline.server() : "localhost";
			cout << "Connecting to '" << u << "'@'" << s << "', with" <<
					(cmdline.pass() && cmdline.pass()[0] ? "" : "out") <<
					" password..." << endl;
		}
		con.connect(0, cmdline.server(), cmdline.user(), cmdline.pass());
	}
	catch (exception& er) {
		cerr << "Connection failed: " << er.what() << endl;
		return 1;
	}
	
	// Create new sample database, or re-create it.  We suppress
	// exceptions, because it's not an error if DB doesn't yet exist.
	bool new_db = false;
	{
		mysqlpp::NoExceptions ne(con);
		mysqlpp::Query query = con.query();
		if (con.select_db(mysqlpp::examples::db_name)) {
			// Toss old tables, ignoring errors because it would just
			// mean the table doesn't exist, which doesn't matter.
			cout << "Dropping existing sample data tables..." << endl;
			query.exec("drop table stock");
			query.exec("drop table images");
			query.exec("drop table deadlock_test1");
			query.exec("drop table deadlock_test2");
		}
		else {
			// Database doesn't exist yet, so create and select it.
			if (con.create_db(mysqlpp::examples::db_name) &&
					con.select_db(mysqlpp::examples::db_name)) {
				new_db = true;
			}
			else {
				cerr << "Error creating DB: " << con.error() << endl;
				return 1;
			}
		}
	}

	// Create sample data table within sample database.
	try {
		// Send the query to create the stock table and execute it.
		cout << "Creating stock table..." << endl;
		mysqlpp::Query query = con.query();
		query << 
				"CREATE TABLE stock (" <<
				"  item CHAR(30) NOT NULL, " <<
				"  num BIGINT NOT NULL, " <<
				"  weight DOUBLE NOT NULL, " <<
				"  price DECIMAL(6,2) NULL, " << // NaN & inf. == NULL
				"  sdate DATE NOT NULL, " <<
				"  description MEDIUMTEXT NULL) " <<
				"ENGINE = InnoDB " <<
				"CHARACTER SET utf8 COLLATE utf8_general_ci";
		query.execute();

		// Set up the template query to insert the data.  The parse()
		// call tells the query object that this is a template and
		// not a literal query string.
		query << "insert into %6:table values " <<
				"(%0q, %1q, %2, %3, %4q, %5q:desc)";
		query.parse();

		// Set a default for template query parameters "table" and "desc".
		query.template_defaults["table"] = "stock";
		query.template_defaults["desc"] = mysqlpp::null;

		// Notice that we don't give a sixth parameter in these calls,
		// so the default value of "stock" is used.  Also notice that
		// the first row is a UTF-8 encoded Unicode string!  All you
		// have to do to store Unicode data in recent versions of MySQL
		// is use UTF-8 encoding.
		cout << "Populating stock table..." << flush;
		query.execute("Nürnberger Brats", 97, 1.5, 8.79, "2005-03-10");
		query.execute("Pickle Relish", 87, 1.5, 1.75, "1998-09-04");
		query.execute("Hot Mustard", 73, .95, .97, "1998-05-25",
				"good American yellow mustard, not that European stuff");
		query.execute("Hotdog Buns", 65, 1.1, 1.1, "1998-04-23");

		// Test that above did what we wanted.
		cout << "inserted " << con.count_rows("stock") << " rows." << endl;

		// Now create empty images table, for testing BLOB and auto-
		// increment column features.
		cout << "Creating empty images table..." << endl;
		query.reset();		// forget template query info
		query << 
				"CREATE TABLE images (" <<
				"  id INT UNSIGNED AUTO_INCREMENT, " <<
				"  data BLOB, " <<
				"  PRIMARY KEY (id)" <<
				")";
		query.execute();

		// Create the tables used by examples/deadlock.cpp
		cout << "Creating deadlock testing tables..." << endl;
		query.execute("CREATE TABLE deadlock_test1 (x INT) ENGINE=innodb");
		query.execute("CREATE TABLE deadlock_test2 (x INT) ENGINE=innodb");
		query.execute("INSERT INTO deadlock_test1 VALUES (1);");
		query.execute("INSERT INTO deadlock_test2 VALUES (2);");

		// Report success
		cout << (new_db ? "Created" : "Reinitialized") <<
				" sample database successfully." << endl;
	}
	catch (const mysqlpp::BadQuery& er) {
		// Handle any query errors
		cerr << endl << "Query error: " << er.what() << endl;
		return 1;
	}
	catch (const mysqlpp::BadConversion& er) {
		// Handle bad conversions
		cerr << endl << "Conversion error: " << er.what() << endl <<
				"\tretrieved data size: " << er.retrieved <<
				", actual size: " << er.actual_size << endl;
		return 1;
	}
	catch (const mysqlpp::Exception& er) {
		// Catch-all for any other MySQL++ exceptions
		cerr << endl << "Error: " << er.what() << endl;
		return 1;
	}

	return 0;
}
