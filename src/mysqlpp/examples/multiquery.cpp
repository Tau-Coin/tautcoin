/***********************************************************************
 multiquery.cpp - Example showing how to iterate over result sets upon
	execution of a query that returns more than one result set.  You can
	get multiple result sets when executing multiple separate SQL
	statments in a single query, or when dealing with the results of
	calling a stored procedure.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB,
 (c) 2004-2009 by Educational Technology Resources, Inc., and (c)
 2005 by Arnon Jalon.  Others may also hold copyrights on code in
 this file.  See the CREDITS.txt file in the top directory of the
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

#include <mysql++.h>

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;
using namespace mysqlpp;


typedef vector<size_t> IntVectorType;


static void
print_header(IntVectorType& widths, StoreQueryResult& res)
{
	cout << "  |" << setfill(' ');
	for (size_t i = 0; i < res.field_names()->size(); i++) {
		cout << " " << setw(widths.at(i)) << res.field_name(int(i)) << " |";
	}
	cout << endl;
}


static void
print_row(IntVectorType& widths, Row& row)
{
	cout << "  |" << setfill(' ');
	for (size_t i = 0; i < row.size(); ++i) {
		cout << " " << setw(widths.at(i)) << row[int(i)] << " |";
	}
	cout << endl;
}


static void
print_row_separator(IntVectorType& widths)
{
	cout << "  +" << setfill('-');
	for (size_t i = 0; i < widths.size(); i++) {
		cout << "-" << setw(widths.at(i)) << '-' << "-+";
	}
	cout << endl;
}


static void
print_result(StoreQueryResult& res, int index)
{
	// Show how many rows are in result, if any
	StoreQueryResult::size_type num_results = res.size();
	if (res && (num_results > 0)) {
		cout << "Result set " << index << " has " << num_results <<
				" row" << (num_results == 1 ? "" : "s") << ':' << endl;
	}
	else {
		cout << "Result set " << index << " is empty." << endl;
		return;
	}

	// Figure out the widths of the result set's columns
	IntVectorType widths;
	size_t size = res.num_fields();
	for (size_t i = 0; i < size; i++) {
		widths.push_back(max(
				res.field(i).max_length(),
				res.field_name(i).size()));
	}

	// Print result set header
	print_row_separator(widths);
	print_header(widths, res);
	print_row_separator(widths);

	// Display the result set contents
	for (StoreQueryResult::size_type i = 0; i < num_results; ++i) {
		print_row(widths, res[i]);
	}

	// Print result set footer
	print_row_separator(widths);
}


static void
print_multiple_results(Query& query)
{
	// Execute query and print all result sets
	StoreQueryResult res = query.store();
	print_result(res, 0);
	for (int i = 1; query.more_results(); ++i) {
		res = query.store_next();
		print_result(res, i);
	}
}


int
main(int argc, char *argv[])
{
	// Get connection parameters from command line
	mysqlpp::examples::CommandLine cmdline(argc, argv);
	if (!cmdline) {
		return 1;
	}

	try {
		// Enable multi-queries.  Notice that you almost always set
		// MySQL++ connection options before establishing the server
		// connection, and options are always set using this one
		// interface.  If you're familiar with the underlying C API,
		// you know that there is poor consistency on these matters;
		// MySQL++ abstracts these differences away.
		Connection con;
		con.set_option(new MultiStatementsOption(true));

		// Connect to the database
		if (!con.connect(mysqlpp::examples::db_name, cmdline.server(),
				cmdline.user(), cmdline.pass())) {
			return 1;
		}

		// Set up query with multiple queries.
		Query query = con.query();
		query << "DROP TABLE IF EXISTS test_table; " <<
				"CREATE TABLE test_table(id INT); " <<
				"INSERT INTO test_table VALUES(10); " <<
				"UPDATE test_table SET id=20 WHERE id=10; " <<
				"SELECT * FROM test_table; " <<
				"DROP TABLE test_table";
		cout << "Multi-query: " << endl << query << endl;

		// Execute statement and display all result sets.
		print_multiple_results(query);

#if MYSQL_VERSION_ID >= 50000
		// If it's MySQL v5.0 or higher, also test stored procedures, which
		// return their results the same way multi-queries do.
		query << "DROP PROCEDURE IF EXISTS get_stock; " <<
				"CREATE PROCEDURE get_stock" <<
				"( i_item varchar(20) ) " <<
				"BEGIN " <<
				"SET i_item = concat('%', i_item, '%'); " <<
				"SELECT * FROM stock WHERE lower(item) like lower(i_item); " <<
				"END;";
		cout << "Stored procedure query: " << endl << query << endl;

		// Create the stored procedure.
		print_multiple_results(query);

		// Call the stored procedure and display its results.
		query << "CALL get_stock('relish')";
		cout << "Query: " << query << endl;
		print_multiple_results(query);
#endif

		return 0;
	}
	catch (const BadOption& err) {
		cerr << err.what() << endl;
		cerr << "This example requires MySQL 4.1.1 or later." << endl;
		return 1;
	}
	catch (const ConnectionFailed& err) {
		cerr << "Failed to connect to database server: " <<
				err.what() << endl;
		return 1;
	}
	catch (const Exception& er) {
		// Catch-all for any other MySQL++ exceptions
		cerr << "Error: " << er.what() << endl;
		return 1;
	}
}
