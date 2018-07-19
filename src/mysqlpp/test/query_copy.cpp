/***********************************************************************
 test/query_copy.cpp - Tests SQL query copies, to ensure that we copy
 	it deeply enough.

 Copyright (c) 2009 by Educational Technology Resources, Inc.
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
using namespace std;


static bool
test_parm(const char* testname, Query& q1, Query q2, const char* parm)
{
	string s1 = q1.str(parm);
	string s2 = q2.str(parm);
	if (s1.compare(s2) == 0) {
		return true;
	}
	else {
		std::cerr << "TEST " << testname << " failed: " <<
				"s1('" << s1 << "') != " <<
				"s2('" << s2 << "')!" << std::endl;
		return false;
	}
}


static bool
test_plain(const char* testname, Query& q1, Query q2)
{
	if (q1.str().compare(q2.str()) == 0) {
		return true;
	}
	else {
		std::cerr << "TEST " << testname << " failed: " <<
				"q1('" << q1.str() << "') != " <<
				"q2('" << q2.str() << "')!" << std::endl;
		return false;
	}
}


int
main()
{
	try {
		Query orig1(0);	// don't pass 0 for conn parameter in real code
		orig1 << "raw string insert method test";
		Query copy1(orig1);
		Query copy2(0); copy2 = orig1;

		Query orig2(0, false, "string ctor test"); // don't do this, either
		Query copy3(orig2);
		Query copy4(0); copy4 = orig2;

		Query orig3(0, false, "template %0 test");
		orig3.parse();
		Query copy5(orig3);
		Query copy6(0); copy6 = orig3;

		if (	test_plain("1a", orig1, copy1) &&
				test_plain("1b", orig1, copy2) &&
				test_plain("2a", orig2, copy3) &&
				test_plain("2b", orig2, copy4) &&
				test_parm("3a", orig3, copy5, "query") &&
				test_parm("3b", orig3, copy6, "query")) {
			return 0;
		}
	}
	catch (const mysqlpp::Exception& e) {
		std::cerr << "Query copy test failed: " << e.what() <<
				std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Unexpected exception: " << e.what() <<
				std::endl;
	}

	return 1;
}

