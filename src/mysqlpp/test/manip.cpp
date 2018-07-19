/***********************************************************************
 test/manip.cpp - Tests the quoting and escaping manipulators.

 Copyright (c) 2007 by Educational Technology Resources, Inc.
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
#include <sstream>
#include <string>


template <class T>
static bool
is_quoted(const std::string& s, T orig_str, size_t orig_len)
{
	return (s.length() == (orig_len + 2)) &&
			(s.at(0) == '\'') &&
			(s.at(orig_len + 1) == '\'') &&
			(s.compare(1, orig_len, orig_str) == 0);
}


template <class T>
static bool
is_quoted(const std::string& s, mysqlpp::Null<T> orig_str, size_t orig_len)
{
	return is_quoted(s, orig_str.data, orig_len);
}


// Stringish types should be quoted when inserted into Query when an
// explicit quote manipulator is used.
template <class T>
static bool
explicit_query_quote(T test, size_t len)
{
	mysqlpp::Query q(0);
	q << mysqlpp::quote << test;
	if (!is_quoted(q.str(), test, len)) {
		std::cerr << "Explicit quote of " << typeid(test).name() <<
				" in Query failed: " << q.str() << std::endl;
		return false;
	}

	mysqlpp::SQLStream s(0);
	s << mysqlpp::quote << test;
	if (is_quoted(s.str(), test, len)) {
		return true;
	}
	else {
		std::cerr << "Explicit quote of " << typeid(test).name() <<
				" in Query failed: " << q.str() << std::endl;
		return false;
	}
}


// Nothing should be quoted when inserted into an ostream, even when an
// explicit quote manipulator is used.  The manipulators are only for
// use with Query streams.
template <class T>
static bool
no_explicit_ostream_quote(T test, size_t len)
{
	std::ostringstream outs;
	outs << mysqlpp::quote << test;
	if (!is_quoted(outs.str(), test, len)) {
		return true;
	}
	else {
		std::cerr << "Explicit quote of " << typeid(test).name() <<
				" in ostream erroneously honored!" << std::endl;
		return false;
	}
}


// Nothing should be implicitly quoted as of v3.  We used to do it for
// mysqlpp::String (formerly ColData) when inserted into Query, but
// that's a silly edge case.  The only time end-user code should be
// using Strings to build queries via the Query stream interface is when
// using BLOBs or when turning result set data back around in a new
// query.  In each case, there's no reason for String to behave
// differently from std::string, which has always had to be explicitly
// quoted.
template <class T>
static bool
no_implicit_quote(T test, size_t len)
{
	std::ostringstream outs;
	outs << test;
	if (!is_quoted(outs.str(), test, len)) {
		mysqlpp::Query q(0);
		q << test;
		if (is_quoted(q.str(), test, len)) {
			std::cerr << typeid(test).name() << " erroneously implicitly "
					"quoted in Query: " << outs.str() <<
					std::endl;
			return false;
		}

		mysqlpp::SQLStream s(0);
		s << test;
		if (!is_quoted(s.str(), test, len)) {
			return true;
		}
		else {
			std::cerr << typeid(test).name() << " erroneously implicitly "
					"quoted in Query: " << outs.str() <<
					std::endl;
			return false;
		}
	}
	else {
		std::cerr << typeid(test).name() << " erroneously implicitly "
				"quoted in ostringstream: " << outs.str() <<
				std::endl;
		return false;
	}
}


// Run all tests above for the given type
template <class T>
static bool
test(T test, size_t len)
{
	return explicit_query_quote(test, len) &&
			no_explicit_ostream_quote(test, len) &&
			no_implicit_quote(test, len);
}


int
main()
{
	char s[] = "Doodle me, James, doodle me!";
	const size_t len = strlen(s);

	int failures = 0;
	failures += test(s, len) == false;
	failures += test(static_cast<char*>(s), len) == false;
	failures += test(static_cast<const char*>(s), len) == false;
	failures += test(std::string(s), len) == false;
	failures += test(mysqlpp::SQLTypeAdapter(s), len) == false;
	failures += test(mysqlpp::Null<std::string>(s), len) == false;
	return failures;
}

