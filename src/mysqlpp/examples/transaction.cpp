/***********************************************************************
 transaction.cpp - Example showing how to use MySQL++'s transaction
 	features.

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
#include <cstdio>

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

		// Show initial state
		mysqlpp::Query query = con.query();
		cout << "Initial state of stock table:" << endl;
		print_stock_table(query);

		// Insert a few rows in a single transaction set
		{
			// Use a higher level of transaction isolation than MySQL
			// offers by default.  This trades some speed for more
			// predictable behavior.  We've set it to affect all
			// transactions started through this DB server connection,
			// so it affects the next block, too, even if we don't
			// commit this one.
			mysqlpp::Transaction trans(con,
					mysqlpp::Transaction::serializable,
					mysqlpp::Transaction::session);

			stock row("Sauerkraut", 42, 1.2, 0.75,
					mysqlpp::sql_date("2006-03-06"), mysqlpp::null);
			query.insert(row);
			query.execute();

			cout << "\nRow inserted, but not committed." << endl;
			cout << "Verify this with another program (e.g. simple1), "
					"then hit Enter." << endl;
			getchar();

			cout << "\nCommitting transaction gives us:" << endl;
			trans.commit();
			print_stock_table(query);
		}
			
		// Now let's test auto-rollback
		{
			// Start a new transaction, keeping the same isolation level
			// we set above, since it was set to affect the session.
			mysqlpp::Transaction trans(con);
			cout << "\nNow adding catsup to the database..." << endl;

			stock row("Catsup", 3, 3.9, 2.99,
					mysqlpp::sql_date("2006-03-06"), mysqlpp::null);
			query.insert(row);
			query.execute();
		}
		cout << "\nNo, yuck! We don't like catsup. Rolling it back:" <<
				endl;
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
