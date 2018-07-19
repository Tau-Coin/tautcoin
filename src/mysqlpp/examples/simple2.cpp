/***********************************************************************
 simple2.cpp - Retrieves the entire contents of the sample stock table
	using a "store" query, and prints it out.

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

#include <iostream>
#include <iomanip>

using namespace std;

int
main(int argc, char *argv[])
{
	// Get database access parameters from command line
	mysqlpp::examples::CommandLine cmdline(argc, argv);
	if (!cmdline) {
		return 1;
	}

	// Connect to the sample database.
	mysqlpp::Connection conn(false);
	if (conn.connect(mysqlpp::examples::db_name, cmdline.server(),
			cmdline.user(), cmdline.pass())) {
		// Retrieve the sample stock table set up by resetdb
		mysqlpp::Query query = conn.query("select * from stock");
		mysqlpp::StoreQueryResult res = query.store();

		// Display results
		if (res) {
			// Display header
			cout.setf(ios::left);
			cout << setw(31) << "Item" <<
					setw(10) << "Num" <<
					setw(10) << "Weight" <<
					setw(10) << "Price" <<
					"Date" << endl << endl;

			// Get each row in result set, and print its contents
			for (size_t i = 0; i < res.num_rows(); ++i) {
				cout << setw(30) << res[i]["item"] << ' ' <<
						setw(9) << res[i]["num"] << ' ' <<
						setw(9) << res[i]["weight"] << ' ' <<
						setw(9) << res[i]["price"] << ' ' <<
						setw(9) << res[i]["sdate"] <<
						endl;
			}
		}
		else {
			cerr << "Failed to get stock table: " << query.error() << endl;
			return 1;
		}

		return 0;
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}
}
