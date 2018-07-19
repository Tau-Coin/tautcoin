/***********************************************************************
 test/qstream.cpp - Tests insertion of all officially-supported data
	types into a Query stream, plus some that aren't official.  Failure
	is defined as an exception being thrown for any one of these.

 Copyright (c) 2008 by Educational Technology Resources, Inc.
 Others may also hold copyrights on code in this file.  See the
 CREDITS.txt file in the top directory of the distribution for details.

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

#include <mysql++.h>

#include <iostream>

using namespace mysqlpp;

int
main()
{
	try {
		// If you're reading this for implicit recommendations of good
		// code style, please ignore the hack-job on the following line.
		mysqlpp::Query q = mysqlpp::Connection().query();

		// Throw everything we can think of at Query's stream interface.
		// Don't do this in real code, either.
		q << 	sql_tinyint(0) << sql_tinyint_unsigned(0) <<
				sql_smallint(0) << sql_smallint_unsigned(0) <<
				sql_mediumint(0) << sql_mediumint_unsigned(0) <<
				sql_int(0) << sql_int_unsigned(0) << long(0) <<
				sql_bigint(0) << sql_bigint_unsigned(0) << longlong(0) <<
				sql_int1(0) << sql_int2(0) << sql_int3(0) <<
				sql_int4(0) << sql_int8(0) << sql_middleint(0) <<
				sql_float(0) << sql_double(0) << sql_decimal(0) <<
				sql_numeric(0) <<
				sql_fixed(0) << sql_float4(0) << sql_float8(0) <<
				sql_bool(false) << sql_boolean(false) << bool(false) <<
				sql_enum() <<
				sql_char() << sql_varchar() << sql_long_varchar() <<
				sql_character_varying() << sql_long() <<
				sql_tinytext() << sql_text() <<
				sql_mediumtext() << sql_longtext() <<
				sql_blob() << sql_tinyblob() << sql_mediumblob() <<
				sql_longblob() << sql_long_varbinary() <<
				sql_date() << sql_time() << sql_datetime() <<
				sql_timestamp() <<
				sql_set();
		std::cout << q << std::endl;
		return 0;
	}
	catch (const mysqlpp::TypeLookupFailed& e) {
		std::cerr << "Query stream insert failed: " << e.what() <<
				std::endl;
		return 1;
	}
	catch (const std::exception& e) {
		std::cerr << "Unexpected exception: " << e.what() <<
				std::endl;
		return 1;
	}
}

