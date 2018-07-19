/***********************************************************************
 test/ssqls2.cpp - Tests the SSQLS v2 mechanism

 Copyright (c) 2009 by Warren Young.  Others may also hold copyrights
 on code in this file.  See the CREDITS.txt file in the top directory
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

#include "../ssx/parsev2.h"

#include <mysql++.h>
#include <ssqls2.h>

#include <iostream>

using namespace std;

// Check that we can create a custom SSQLS v2 subclass by hand.  Tests
// for unexpected changes in SsqlsBase definition.
class TestSubclass : public mysqlpp::SsqlsBase
{
public:
	TestSubclass() :
	mysqlpp::SsqlsBase()
	{
	}

	bool create_table(mysqlpp::Connection* conn = 0) const 
			{ (void)conn; return false; }
	std::ostream& equal_list(std::ostream& os,
			FieldSubset fs = fs_set) const
			{ (void)fs; return os; }
	std::ostream& name_list(std::ostream& os,
			FieldSubset fs = fs_set) const 
			{ (void)fs; return os; }
	bool populated(FieldSubset fs = fs_all) const
			{ (void)fs; return false; }
	std::ostream& value_list(std::ostream& os,
			FieldSubset fs = fs_set) const 
			{ (void)fs; return os; }
};


// Test a single string to ParseV2::Field::Type value conversion
static bool
Test(const char* input_type, const ParseV2::Field::Type& expected_value)
{
	ParseV2::Field::Type actual_value(input_type);
	if (actual_value == expected_value) {
		return true;
	}
	else {
		std::cerr << '"' << input_type << "\" converted to " <<
				int(actual_value) << " not " <<
				int(expected_value) << " as expected!" << std::endl;
		return false;
	}
}


// Test as many string to ParseV2::Field::Type value conversions as we
// expect the smart enum to support.
static bool
TestFieldTypeConversions()
{
	return 
		// First, test all the known "sane" inputs
		Test("bigblob", ParseV2::Field::Type::ft_blob) &&
		Test("bigint", ParseV2::Field::Type::ft_bigint) &&
		Test("bigtext", ParseV2::Field::Type::ft_string) &&
		Test("blob", ParseV2::Field::Type::ft_blob) &&
		Test("bool", ParseV2::Field::Type::ft_tinyint) &&
		Test("boolean", ParseV2::Field::Type::ft_tinyint) &&
		Test("char", ParseV2::Field::Type::ft_string) &&
		Test("date", ParseV2::Field::Type::ft_date) &&
		Test("datetime", ParseV2::Field::Type::ft_datetime) &&
		Test("decimal", ParseV2::Field::Type::ft_double) &&
		Test("double", ParseV2::Field::Type::ft_double) &&
		Test("enum", ParseV2::Field::Type::ft_string) &&
		Test("fixed", ParseV2::Field::Type::ft_double) &&
		Test("float", ParseV2::Field::Type::ft_float) &&
		Test("float4", ParseV2::Field::Type::ft_float) &&
		Test("float8", ParseV2::Field::Type::ft_float) &&
		Test("int", ParseV2::Field::Type::ft_mediumint) &&
		Test("int1", ParseV2::Field::Type::ft_tinyint) &&
		Test("int2", ParseV2::Field::Type::ft_smallint) &&
		Test("int3", ParseV2::Field::Type::ft_mediumint) &&
		Test("int4", ParseV2::Field::Type::ft_mediumint) &&
		Test("int8", ParseV2::Field::Type::ft_bigint) &&
		Test("mediumblob", ParseV2::Field::Type::ft_blob) &&
		Test("mediumint", ParseV2::Field::Type::ft_mediumint) &&
		Test("mediumtext", ParseV2::Field::Type::ft_string) &&
		Test("numeric", ParseV2::Field::Type::ft_double) &&
		Test("set", ParseV2::Field::Type::ft_set) &&
		Test("smallblob", ParseV2::Field::Type::ft_blob) &&
		Test("smallint", ParseV2::Field::Type::ft_smallint) &&
		Test("smalltext", ParseV2::Field::Type::ft_string) &&
		Test("text", ParseV2::Field::Type::ft_string) &&
		Test("time", ParseV2::Field::Type::ft_time) &&
		Test("timestamp", ParseV2::Field::Type::ft_datetime) &&
		Test("tinyblob", ParseV2::Field::Type::ft_blob) &&
		Test("tinyint", ParseV2::Field::Type::ft_tinyint) &&
		Test("tinytext", ParseV2::Field::Type::ft_string) &&
		Test("varbinary", ParseV2::Field::Type::ft_blob) &&
		Test("varchar", ParseV2::Field::Type::ft_string) &&
		// Test that it's properly case-insensitive
		Test("Numeric", ParseV2::Field::Type::ft_double) &&
		Test("sEt", ParseV2::Field::Type::ft_set) &&
		Test("SMALLBLOB", ParseV2::Field::Type::ft_blob) &&
		// Test that mildly bogus conversions are handled sanely
		Test("char(8)", ParseV2::Field::Type::ft_string) &&
		Test("decimal(16)", ParseV2::Field::Type::ft_double) &&
		Test("int5", ParseV2::Field::Type::ft_mediumint) &&
		Test("varchar(32)", ParseV2::Field::Type::ft_string) &&
		// Test that truly bogus stuff gets treated as stringish
		Test("varwhatsit(64)", ParseV2::Field::Type::ft_string) &&
		Test("klH54%KJgh^7hh4jvwtt", ParseV2::Field::Type::ft_string) &&
		true;
}


int
main()
{
	// Force instantiation of custom subclass
	TestSubclass tsc;

	return TestFieldTypeConversions() ? 0 : 1;
}
