/***********************************************************************
 tquery4.cpp - Tests other details about template queries, like unquoted
	parameters, multiple parameters, and preventing problems with LIKE
	patterns.  This exists more for code coverage than to demonstrate
	the library.

 Copyright (c) 2009 by Martin Gallwey and (c) 2009 by Educational
 Technology Resources, Inc.  Others may also hold copyrights on code
 in this file.  See the CREDITS.txt file in the top directory of the
 distribution for details.

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

		// Modify an item using two named template query parameters
		mysqlpp::Query query = con.query("update stock "
				"set num = %0:quantity where num < %0:quantity");
		query.parse();
		query.template_defaults["quantity"] = 70;
		cout << "Query: " << query << endl;
		mysqlpp::SimpleResult result = query.execute();

		// Print the new table contents.
		print_stock_table(query);

		// Now let's check multiple dissimilar parameter types, and show
		// how to avoid conflicts between '%' as used in tqueries vs in
		// LIKE patterns.
		query.reset();
		query << "select * from stock where weight > %0q or "
				"description like '%%%1%%'";
		query.parse();
		cout << "\nQuery: " << query.str(1.2, "Mustard") << endl; 
		mysqlpp::StoreQueryResult res = query.store(1.2, "Mustard"); 

		// Show what second tquery found
		print_stock_rows(res);
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
