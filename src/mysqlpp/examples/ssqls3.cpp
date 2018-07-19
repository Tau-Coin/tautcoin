/***********************************************************************
 ssqls3.cpp - Example showing how to update an SQL row using the
	Specialized SQL Structures feature of MySQL++.

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
#include "stock.h"

#include <iostream>

using namespace std;

int
main(int argc, char *argv[])
{
	// Get database access parameters from command line
	mysqlpp::examples::CommandLine cmdline(argc, argv);
	if (!cmdline) {
		return 1;
	}

	try {
		// Establish the connection to the database server.
		mysqlpp::Connection con(mysqlpp::examples::db_name,
				cmdline.server(), cmdline.user(), cmdline.pass());

		// Build a query to retrieve the stock item that has Unicode
		// characters encoded in UTF-8 form.
		mysqlpp::Query query = con.query("select * from stock ");
		query << "where item = " << mysqlpp::quote << "Nürnberger Brats";

		// Retrieve the row, throwing an exception if it fails.
		mysqlpp::StoreQueryResult res = query.store();
		if (res.empty()) {
			throw mysqlpp::BadQuery("UTF-8 bratwurst item not found in "
					"table, run resetdb");
		}

		// Because there should only be one row in the result set,
		// there's no point in storing the result in an STL container.
		// We can store the first row directly into a stock structure
		// because one of an SSQLS's constructors takes a Row object.
		stock row = res[0];

		// Create a copy so that the replace query knows what the
		// original values are.
		stock orig_row = row;

		// Change the stock object's item to use only 7-bit ASCII, and
		// to deliberately be wider than normal column widths printed
		// by print_stock_table().
		row.item = "Nuerenberger Bratwurst";

		// Form the query to replace the row in the stock table.
		query.update(orig_row, row);

		// Show the query about to be executed.
		cout << "Query: " << query << endl;

		// Run the query with execute(), since UPDATE doesn't return a
		// result set.
		query.execute();

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
	catch (const mysqlpp::Exception& er) {
		// Catch-all for any other MySQL++ exceptions
		cerr << "Error: " << er.what() << endl;
		return -1;
	}

	return 0;
}
