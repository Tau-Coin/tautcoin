/***********************************************************************
 ssqls4.cpp - Example very similar to ssqls1.cpp, except that it
	stores its result set in an STL set container.  This demonstrates
	how one can manipulate MySQL++ result sets in a very natural C++
	style.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB, and
 (c) 2004-2010 by Educational Technology Resources, Inc.  Others may
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

		// Retrieve all rows from the stock table and put them in an
		// STL set.  Notice that this works just as well as storing them
		// in a vector, which we did in ssqls1.cpp.  It works because
		// SSQLS objects are less-than comparable.
		mysqlpp::Query query = con.query("select * from stock");
		set<stock> res;
		query.storein(res);

		// Display the result set.  Since it is an STL set and we set up
		// the SSQLS to compare based on the item column, the rows will
		// be sorted by item.
		print_stock_header(res.size());
		set<stock>::iterator it;
		cout.precision(3);
		for (it = res.begin(); it != res.end(); ++it) {
			print_stock_row(it->item.c_str(), it->num, it->weight,
					it->price, it->sDate);
		}

		// Use set's find method to look up a stock item by item name.
		// This also uses the SSQLS comparison setup.
		it = res.find(stock("Hotdog Buns"));
		if (it != res.end()) {
			cout << endl << "Currently " << it->num <<
					" hotdog buns in stock." << endl;
		}
		else {
			cout << endl << "Sorry, no hotdog buns in stock." << endl;
		}
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
