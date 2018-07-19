/***********************************************************************
 for_each.cpp - Demonstrates Query::for_each(), showing how to perform
	an arbitrary action on each row in a result set.

 Copyright (c) 2005-2009 by Educational Technology Resources, Inc. and
 (c) 2007 by Switchplane, Ltd.  Others may also hold copyrights on
 code in this file.  See the CREDITS.txt file in the top directory
 of the distribution for details.

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

#include <mysql++.h>

#include <iostream>

#include <math.h>


// Define a functor to collect statistics about the stock table
class gather_stock_stats
{
public:
	gather_stock_stats() :
	items_(0),
	weight_(0),
	cost_(0)
	{
	}

	void operator()(const stock& s)
	{
		items_  += s.num;
		weight_ += (s.num * s.weight);
		cost_   += (s.num * s.price.data);
	}
	
private:
	mysqlpp::sql_bigint items_;
	mysqlpp::sql_double weight_, cost_;

	friend std::ostream& operator<<(std::ostream& os,
			const gather_stock_stats& ss);
};


// Dump the contents of gather_stock_stats to a stream in human-readable
// form.
std::ostream&
operator<<(std::ostream& os, const gather_stock_stats& ss)
{
	os << ss.items_ << " items " <<
			"weighing " << ss.weight_ << " stone and " <<
			"costing " << ss.cost_ << " cowrie shells";
	return os;
}


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

		// Gather and display the stats for the entire stock table
		mysqlpp::Query query = con.query();
		std::cout << "There are " << query.for_each(stock(),
				gather_stock_stats()) << '.' << std::endl;
	}
	catch (const mysqlpp::BadQuery& e) {
		// Something went wrong with the SQL query.
		std::cerr << "Query failed: " << e.what() << std::endl;
		return 1;
	}
	catch (const mysqlpp::Exception& er) {
		// Catch-all for any other MySQL++ exceptions
		std::cerr << "Error: " << er.what() << std::endl;
		return 1;
	}

	return 0;
}
