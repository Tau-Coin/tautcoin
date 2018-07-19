/***********************************************************************
 test/qssqls.cpp - Tests SQL query creation from SSQLS in Query.

 Copyright (c) 2008-2009 by Educational Technology Resources, Inc.
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
#define MYSQLPP_ALLOW_SSQLS_V1	// suppress deprecation warning
#include <ssqls.h>

#include <iostream>

using namespace mysqlpp;
using namespace std;


sql_create_19(test,
	19, 0,
	sql_tinyint,			tinyint_v,
	sql_tinyint_unsigned,	tinyint_unsigned_v,
	sql_smallint,			smallint_v,
	sql_smallint_unsigned,	smallint_unsigned_v,
	sql_int,				int_v,
	sql_int_unsigned,		int_unsigned_v,
	sql_mediumint,			mediumint_v,
	sql_mediumint_unsigned, mediumint_unsigned_v,
	sql_bigint,				bigint_v,
	sql_bigint_unsigned,	bigint_unsigned_v,
	sql_float,				float_v,
	sql_double,				double_v,
	sql_decimal,			decimal_v,
	sql_bool,				bool_v,
	sql_date,				date_v,
	sql_time,				time_v,
	sql_datetime,			datetime_v,
	sql_char,				char_v,	// only need one stringish type...
	sql_blob,				blob_v)	// ...and one blob type; they're all
									// the same under the hood in MySQL++

int
main()
{
	Query q(0);		// don't pass 0 for conn parameter in real code
	test empty(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false,
			Date(), Time(), DateTime(), "", sql_blob());
	test filled(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11.0, 12.0, 13.0,
			bool(14), Date("1515-15-15"), Time("16:16:16"),
			DateTime("1717-17-17 17:17:17"), "18", sql_blob("1\09", 3));

	cout << q.insert(empty) << endl << endl;
	cout << q.insert(filled) << endl << endl;
	cout << q.replace(empty) << endl << endl;
	cout << q.replace(filled) << endl << endl;
	cout << q.update(filled, empty) << endl << endl;

	return 0;
}

