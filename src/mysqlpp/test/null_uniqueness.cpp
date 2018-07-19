/***********************************************************************
 test/null_uniqueness.cpp - Code for checking that null_type cannot be
	converted to anything else.  Because it triggers a compile-time
	check, it can't be included in the test suite.  You have to just
	try building it.  Comment out the assignment to int to check that
	the return statement also triggers the compile-time check.

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

int
main()
{
	mysqlpp::Null<int> ni = mysqlpp::null;
	int this_should_not_even_compile = mysqlpp::null;
	return ni;		// neither should this
}

