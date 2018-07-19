/***********************************************************************
 ssqls6.cpp - Example showing how to insert a collection row using the 
 Specialized SQL Structures feature of MySQL++ and Query::insertfrom().

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB,
 (c) 2004-2009 by Educational Technology Resources, Inc., (c) 2008 by 
 AboveNet, Inc.  Others may also hold copyrights on code in this file.  
 See the CREDITS file in the top directory of the distribution for details.

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
#include "stock.h"

#include <fstream>

using namespace std;


// Breaks a given text line of tab-separated fields up into a list of
// strings.
static size_t
tokenize_line(const string& line, vector<mysqlpp::String>& strings)
{
	string field;
	strings.clear();

	istringstream iss(line);
	while (getline(iss, field, '\t')) {
		strings.push_back(mysqlpp::String(field));
	}

	return strings.size();
}


// Reads a tab-delimited text file, returning the data found therein
// as a vector of stock SSQLS objects.
static bool
read_stock_items(const char* filename, vector<stock>& stock_vector)
{
	ifstream input(filename);
	if (!input) {
		cerr << "Error opening input file '" << filename << "'" << endl;
		return false;
	}

	string line;
	vector<mysqlpp::String> strings;
	while (getline(input, line)) {
		if (tokenize_line(line, strings) == 6) {
			stock_vector.push_back(stock(string(strings[0]), strings[1],
					strings[2], strings[3], strings[4], strings[5]));
		}
		else {
			cerr << "Error parsing input line (doesn't have 6 fields) " << 
					"in file '" << filename << "'" << endl;
			cerr << "invalid line: '" << line << "'" << endl;
		}
	}

	return true;
}


int
main(int argc, char *argv[])
{
	// Get database access parameters from command line
	mysqlpp::examples::CommandLine cmdline(argc, argv);
	if (!cmdline) {
		return 1;
	}

	// Read in a tab-delimited file of stock data
	vector<stock> stock_vector;
	if (!read_stock_items("examples/stock.txt", stock_vector)) {
		return 1;
	}

	try {
		// Establish the connection to the database server.
		mysqlpp::Connection con(mysqlpp::examples::db_name,
				cmdline.server(), cmdline.user(), cmdline.pass());

		// Clear all existing rows from stock table, as we're about to
		// insert a bunch of new ones, and we want a clean slate.
		mysqlpp::Query query = con.query();
		query.exec("DELETE FROM stock");

		// Insert data read from the CSV file, allowing up to 1000
		// characters per packet.  We're using a small size in this
		// example just to force multiple inserts.  In a real program,
		// you'd want to use larger packets, for greater efficiency.
		mysqlpp::Query::MaxPacketInsertPolicy<> insert_policy(1000);
		query.insertfrom(stock_vector.begin(), stock_vector.end(),
				insert_policy);

		// Retrieve and print out the new table contents.
		print_stock_table(query);
	}
	catch (const mysqlpp::BadQuery& er) {
		// Handle any query errors
		cerr << "Query error: " << er.what() << endl;
		return -1;
	}
	catch (const mysqlpp::BadConversion& er) {
		// Handle bad conversions
		cerr << "Conversion error: " << er.what() << endl <<
				"\tretrieved data size: " << er.retrieved <<
				", actual size: " << er.actual_size << endl;
		return -1;
	}
	catch (const mysqlpp::BadInsertPolicy& er) {
		// Handle bad conversions
		cerr << "InsertPolicy error: " << er.what() << endl;
		return -1;
	}
	catch (const mysqlpp::Exception& er) {
		// Catch-all for any other MySQL++ exceptions
		cerr << "Error: " << er.what() << endl;
		return -1;
	}

	return 0;
}

