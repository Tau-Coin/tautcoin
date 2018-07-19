/***********************************************************************
 test/null_comparison.cpp - Tests that Null<T> and null_type comparison
	operators and SSQLS comparison functions work correctly.

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

sql_create_1(ssqls, 1, 0, mysqlpp::Null<int>, a_column)

int
main()
{
	mysqlpp::Null<int> null_int = mysqlpp::null;
	mysqlpp::Null<int> non_null_int = 42;
	if (	!(null_int == non_null_int) &&
			(null_int != non_null_int) &&
			(null_int < non_null_int) &&
			!(non_null_int == null_int) &&
			(non_null_int != null_int) &&
			!(non_null_int < null_int) &&
			(null_int == mysqlpp::null) &&
			!(null_int != mysqlpp::null) &&
			(non_null_int != mysqlpp::null) &&
			!(non_null_int == mysqlpp::null) &&
			(mysqlpp::sql_cmp(null_int, null_int) == 0) &&
			(mysqlpp::sql_cmp(null_int, non_null_int) < 0) &&
			(mysqlpp::sql_cmp(non_null_int, null_int) > 0)) {
		ssqls foo(null_int), bar(non_null_int);
		if ((foo < bar) && (foo != bar) && !(bar < foo) && !(foo == bar)) {
			return 0;
		}
		else {
			std::cerr << "SSQLS comparison gave unexpected result" <<
					std::endl;
			return 1;
		}
	}
	else {
		std::cerr << "Null comparison gave unexpected result" << std::endl;
		return 1;
	}
}

