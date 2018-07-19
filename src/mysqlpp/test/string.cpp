/***********************************************************************
 test/string.cpp - Tests the behavior of mysqlpp::String, particularly
	its data conversion methods.

 Copyright (c) 2007-2008 by Educational Technology Resources, Inc.
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


// Does an equality comparison on the value, forcing the string to
// convert itself to T on the way.  Note that we do this test in terms
// of greater and less than to avoid pedantic GCC warnings for the
// floating point type tests.
template <typename T>
static bool
test_equality(const mysqlpp::String& s, T value)
{
	T converted = s.conv(value);
	if ((value < converted) || (value > converted)) {
		std::cerr << "Type conversion to " << typeid(T).name() <<
				" failed: \"" << s << "\" != \"" << value << "\"." <<
				std::endl;
		return false;
	}
	else {
		return true;
	}
}


// Check that we can convert strings with decimals in them to native
// floating-point values, regardless of locale.
static bool
test_float_conversion()
{
	// This stuff should just work
	if (!test_equality(mysqlpp::String("123.00"), 123)) return false;
	if (!test_equality(mysqlpp::String("123."), 123)) return false;

	// This is trickier: MySQL ignores the system locale when it comes
	// to decimal separators, always using '.', so ensure the conversion
	// stuff in MySQL++ does the right thing regardless.  Test against
	// this system's current locale, an arbitrary European one where ','
	// is the decimal separator, and the "C" locale where it's '.'.
	if (!test_equality(mysqlpp::String("621.200"), 621.2)) return false;
	std::locale old_locale = std::locale::global(std::locale::classic());
	if (!test_equality(mysqlpp::String("621.200"), 621.2)) return false;
	try {
		std::locale::global(std::locale("de_DE"));
		if (!test_equality(mysqlpp::String("621.200"), 621.2)) return false;
	}
	catch (std::runtime_error& e) {
		std::cerr << "WARNING: skipping European locale string "
				"conversion test:" << std::endl;
		std::cerr << "\t" << e.what() << std::endl;
	}
	std::locale::global(old_locale);

	// Check that we choke on silly float-like values
	try {
		if (test_equality(mysqlpp::String("621.20.0"), 621.2)) {
			std::cerr << "Quasi-FP with two decimal points "
					"converting without error!" << std::endl;
		}
		return false;
	}
	catch (const mysqlpp::BadConversion&) {
		return true;
	}
}


// Tries to convert the given string to an int.  Returns false if we got
// a BadConversion exception and didn't expect it, or didn't get one we
// expected.  Returns false on all other exceptions regardless.
static bool
test_int_conversion(const mysqlpp::String& s, bool throw_expected)
{
	// Try the conversion
	bool conv_threw = false;
	try {
		int converted = s;
		(void)converted;	// pedantic warning squisher
	}
	catch (const mysqlpp::BadConversion&) {
		conv_threw = true;
	}
	catch (const std::exception& e) {
		std::cerr << "Unexpected " << typeid(e).name() <<
				" exception in test_int_conv: " << e.what() << std::endl;
		return false;
	}
	catch (...) {
		std::cerr << "Like, totally bogus exception in test_int_conv, "
				"man!" << std::endl;
		return false;
	}

	// Did it do what we expected?
	if (throw_expected == conv_threw) {
		return true;
	}
	else {
		std::cerr << "Conversion of \"" << s << "\" to int " <<
				(conv_threw ? "did not throw" : "threw") << "; " <<
				(throw_expected ? "did not expect" : "expected") <<
				" it to." << std::endl;
		return false;
	}
}


// Checks that String's null comparison methods work right
static bool
test_null()
{
	mysqlpp::String not_null("", mysqlpp::mysql_type_info::string_type, false);
	mysqlpp::String is_null("", mysqlpp::mysql_type_info::string_type, true);
	if (not_null.is_null() == true) {
		std::cerr << "not_null.is_null() == true!" << std::endl;
		return false;
	}
	else if (not_null == mysqlpp::null) {
		std::cerr << "not_null == mysqlpp:null!" << std::endl;
		return false;
	}
	else if (is_null.is_null() == false) {
		std::cerr << "is_null.is_null() == false!" << std::endl;
		return false;
	}
	else if (is_null != mysqlpp::null) {
		std::cerr << "is_null != mysqlpp:null!" << std::endl;
		return false;
	}
	else {
		return true;
	}
}


// Ensures numeric conversions of many different types get handled
// correctly.
static bool
test_numeric(const mysqlpp::String& s, int value)
{
	return	test_equality(s, static_cast<signed char>(value)) &&
			test_equality(s, static_cast<unsigned char>(value)) &&
			test_equality(s, static_cast<signed short>(value)) &&
			test_equality(s, static_cast<unsigned short>(value)) &&
			test_equality(s, static_cast<signed int>(value)) &&
			test_equality(s, static_cast<unsigned int>(value)) &&
			test_equality(s, static_cast<signed long>(value)) &&
			test_equality(s, static_cast<unsigned long>(value)) &&
#if !defined(NO_LONG_LONGS)
			test_equality(s, static_cast<mysqlpp::longlong>(value)) &&
			test_equality(s, static_cast<mysqlpp::ulonglong>(value)) &&
#endif
			test_equality(s, static_cast<float>(value)) &&
			test_equality(s, static_cast<double>(value));
}


static bool
test_quote_q(const mysqlpp::String& s, bool expected)
{
	if (s.quote_q() == expected) {
		return true;
	}
	else {
		std::cerr << s.type().name() << " should" <<
				(expected ? "" : " NOT") << " be quoted." << std::endl;
		return false;
	}
}


// Similar to test_equality, but only works with std::string
// comparisons, which uses String::operator ==()
static bool
test_string_equality(const mysqlpp::String& s, std::string value)
{
	if (s == value) {
		if (s != value) {
			std::cerr << "String(\"" << s << "\") != std::string(\"" <<
					value << "\"), case 2!" << std::endl;
			return false;
		}
		else {
			return true;
		}
	}
	else {
		std::cerr << "String(\"" << s << "\") != std::string(\"" <<
				value << "\"), case 1!" << std::endl;
		return false;
	}
}


// Same as above, but for two String objects
static bool
test_string_equality(const mysqlpp::String& s1,
		const mysqlpp::String s2)
{
	if (s1 == s2) {
		return true;
	}
	else {
		std::cerr << "String(\"" << s1 << "\") != String(\"" <<
				s2 << "\"), but should be equal!" << std::endl;
		return false;
	}
}


// Inverse of above.
static bool
test_string_inequality(const mysqlpp::String& s1,
		const mysqlpp::String& s2)
{
	if (s1 != s2) {
		return true;
	}
	else {
		std::cerr << "String(\"" << s1 << "\") == String(\"" <<
				s2 << "\") but shouldn't be!" << std::endl;
		return false;
	}
}


int
main(int, char* argv[])
{
	try {
		int failures = 0;
		mysqlpp::String definit;
		mysqlpp::String empty("");
		mysqlpp::String zero("0");
		mysqlpp::String nonzero("42");
		mysqlpp::String intable1("42.");
		mysqlpp::String intable2("42.0");
		mysqlpp::String nonint("42.1");

		failures += test_equality(definit, mysqlpp::Date()) == false;
		failures += test_equality(definit,
				mysqlpp::DateTime(0, 0, 0, 0, 0, 0)) == false;
		failures += test_equality(definit, mysqlpp::Time()) == false;
		failures += test_equality(definit, false) == false;
		failures += test_equality(nonzero, true) == false;
		failures += test_numeric(definit, 0) == false;
		failures += test_numeric(zero, 0) == false;
		failures += test_numeric(nonzero, 42) == false;
		failures += test_quote_q(definit, true) == false;
		failures += test_quote_q(mysqlpp::String("1", typeid(int)),
				false) == false;
		failures += test_float_conversion() == false;
		failures += test_float_conversion() == false;
		failures += test_int_conversion(definit, false) == false;
		failures += test_int_conversion(zero, false) == false;
		failures += test_int_conversion(nonzero, false) == false;
		failures += test_int_conversion(intable1, false) == false;
		failures += test_int_conversion(intable2, false) == false;
		failures += test_int_conversion(nonint, true) == false;
		failures += test_null() == false;
		failures += test_string_equality(definit, empty) == false;
		failures += test_string_equality(empty, definit) == false;
		failures += test_string_equality(definit, "") == false;
		failures += test_string_equality(zero, "0") == false;
		failures += test_string_inequality(definit, zero) == false;
		failures += test_string_inequality(zero, definit) == false;
		failures += test_string_inequality(empty, nonzero) == false;
		
		return failures;
	}
	catch (mysqlpp::Exception& e) {
		std::cerr << "Unexpected MySQL++ exception caught in " <<
				argv[0] << ": " << e.what() << std::endl;
		return 1;
	}
	catch (std::exception& e) {
		std::cerr << "Unexpected C++ exception caught in " <<
				argv[0] << ": " << e.what() << std::endl;
		return 1;
	}

}

