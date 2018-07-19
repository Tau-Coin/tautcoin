/***********************************************************************
 test/array_index.cpp - Tests operator[] and at() on indexable objects
	to ensure they throw exceptions when given bad indices.

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


template <class ContainerT>
static bool
test_exception(const ContainerT& container, int index)
{
	try {
		container[index];
		container.at(index);
		std::cerr << "Bad index " << index << " allowed in " <<
				typeid(container).name() << '!' << std::endl;
		return false;
	}
	catch (const mysqlpp::BadIndex&) {
		return true;
	}
	catch (const mysqlpp::Exception& e) {
		std::cerr << "Unexpected mysqlpp::Exception caught for " <<
				typeid(container).name() << ", index " << index <<
				": " << e.what() << std::endl;
		return false;
	}
	catch (const std::exception& e) {
		std::cerr << "Unexpected std::exception caught for " <<
				typeid(container).name() << ", index " << index <<
				": " << e.what() << std::endl;
		return false;
	}
	catch (...) {
		std::cerr << "Unexpected exception type caught for " <<
				typeid(container).name() << ", index " << index <<
				'!' << std::endl;
		return false;
	}
}


template <class ContainerT>
static bool
test_no_exception(const ContainerT& container)
{
	mysqlpp::NoExceptions ne(container);

	try {
		container[-1];
		std::cerr << "Mandatory exception suppressed for " <<
				typeid(container).name() << "::operator[]()!" <<
				std::endl;
		return false;
	}
	catch (...) {
	}

	try {
		container.at(-1);
		std::cerr << "Mandatory exception suppressed for " <<
				typeid(container).name() << "::at()!" << std::endl;
		return false;
	}
	catch (...) {
		return true;
	}
}


// Separate test needed because string indices work different from
// integer indices.  Exceptions for bad field names *can* be suppressed;
// SSQLS needs this to support population of partial structures, with
// the un-queried fields getting default values.
template <class ContainerT>
static bool
test_string_index(const ContainerT& container)
{
	// This test should cause an exception...
	try {
		container["fred"];
		std::cerr << "Bad string index allowed in " <<
				typeid(container).name() << '!' << std::endl;
		return false;
	}
	catch (const mysqlpp::BadFieldName&) {
		// Good; fall through to next test
	}
	catch (const mysqlpp::Exception& e) {
		std::cerr << "Unexpected mysqlpp::Exception caught for "
				"bad string index in " << typeid(container).name() <<
				": " << e.what() << std::endl;
		return false;
	}
	catch (const std::exception& e) {
		std::cerr << "Unexpected std::exception caught for "
				"bad string index in " << typeid(container).name() <<
				": " << e.what() << std::endl;
		return false;
	}
	catch (...) {
		std::cerr << "Unexpected exception type caught for "
				"bad string index in " << typeid(container).name() <<
				'!' << std::endl;
		return false;
	}

	// ...but not this one
	mysqlpp::NoExceptions ne(container);
	try {
		container["fred"];
		return true;
	}
	catch (const mysqlpp::BadFieldName&) {
		std::cerr << "Exception not suppressed for nonexistent field "
				"in " << typeid(container).name() << '!' << std::endl;
		return false;
	}
}


template <class ContainerT>
static bool
test_numeric_index(const ContainerT& container)
{
	return	test_exception(container, -1) &&
			test_exception(container, 0) &&
			test_exception(container, 1);
}


int
main()
{
	try {
		return	test_no_exception(mysqlpp::Row()) &&
				test_numeric_index(mysqlpp::Row()) &&
				test_string_index(mysqlpp::Row()) ? 0 : 1;
	}
	catch (...) {
		std::cerr << "Unhandled exception caught by array_index!" <<
				std::endl;
		return 2;
	}
}

