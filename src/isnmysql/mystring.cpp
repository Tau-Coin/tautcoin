/***********************************************************************
 mystring.cpp - Implements the String class.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB, and
 (c) 2004-2008 by Educational Technology Resources, Inc.  Others may
 also hold copyrights on code in this file.  See the CREDITS.txt file
 in the top directory of the distribution for details.

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

#include "mystring.h"
#include "query.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace mysqlpp {


char
String::at(size_type pos) const
{
	if (pos >= size()) {
		throw BadIndex("String", int(pos), int(size()));
	}
	else {
		return buffer_->data()[pos];
	}
}


int
String::compare(const String& other) const
{
	if (other.buffer_) {
		return compare(0, std::max(length(), other.length()),
				other.buffer_->data());
	}
	else {
		// Other object has no buffer, so we are greater unless empty or
		// we also have no buffer.
		return length() > 0 ? 1 : 0;	
	}
}


int
String::compare(const std::string& other) const
{
	return compare(0, std::max(length(), other.length()), other.data());
}


int
String::compare(size_type pos, size_type num, std::string& other) const
{
	return compare(pos, num, other.data());
}


int
String::compare(const char* other) const
{
	return compare(0, std::max(length(), strlen(other)), other);
}


int
String::compare(size_type pos, size_type num,
		const char* other) const
{
	if (buffer_ && other) {
		return strncmp(data() + pos, other, num);
	}
	else if (!other) {
		// Initted and non-empty is "greater than" uninitted
		return length() > 0 ? 1 : 0;
	}
	else {
		// This object has no buffer, so we are less than other object
		// unless it is empty.
		return other[0] == '\0' ? 0 : -1;
	}
}


#if !defined(DOXYGEN_IGNORE)
// Doxygen isn't smart enough to recognize these template
// specializations.  Maybe it's the MYSQLPP_EXPORT tags?

template <>
String
String::conv(String) const { return *this; }


template <>
bool
String::conv(bool) const
{
	return *this;	// delegate to operator bool
}


template <>
std::string
String::conv(std::string) const
{
	return buffer_ ? std::string(data(), length()) : std::string();
}


template <>
Date
String::conv(Date) const
{
	return buffer_ ? Date(c_str()) : Date();
}


template <>
DateTime
String::conv(DateTime) const
{
	return buffer_ ? DateTime(c_str()) : DateTime();
}


template <>
Time
String::conv(Time) const
{
	return buffer_ ? Time(c_str()) : Time();
}

#endif // !defined(DOXYGEN_IGNORE)


const char*
String::data() const
{
	return buffer_ ? buffer_->data() : 0;
}


String::const_iterator
String::end() const
{
	return buffer_ ? buffer_->data() + buffer_->length() : 0;
}


bool
String::escape_q() const
{
	return buffer_ ? buffer_->type().escape_q() : false;
}


bool
String::is_null() const
{
	return buffer_ ? buffer_->is_null() : false;
}


void
String::it_is_null()
{
	if (buffer_) {
		buffer_->set_null();
	}
	else {
		buffer_ = new SQLBuffer(0, 0, mysql_type_info::string_type, true);
	}
}


String::size_type
String::length() const
{
	return buffer_ ? buffer_->length() : 0;
}


bool
String::quote_q() const
{
	// If no buffer, it means we're an empty string, so we need to be 
	// quoted to be expressed properly in SQL.
	return buffer_ ? buffer_->type().quote_q() : true;
}


void
String::to_string(std::string& s) const
{
	if (buffer_) {
		s.assign(buffer_->data(), buffer_->length());
	}
	else {
		s.clear();
	}
}


/// \brief Stream insertion operator for String objects
///
/// This doesn't have anything to do with the automatic quoting and
/// escaping you get when using SQLTypeAdapter with Query.  The need to
/// use String with Query should be rare, since String generally comes
/// in result sets; it should only go back out as queries when using
/// result data in a new query.  Since SQLTypeAdapter has a conversion
/// ctor for String, this shouldn't be a problem.  It's just trading
/// simplicity for a tiny bit of inefficiency in a rare case.  And 
/// since String and SQLTypeAdapter can share a buffer, it's not all
/// that inefficient anyway.

std::ostream&
operator <<(std::ostream& o, const String& in)
{
	if (dynamic_cast<Query*>(&o)) {
		// We can just insert the raw data into the stream
		o.write(in.data(), in.length());
	}
	else {
		// Can't guess what sort of stream it is, so convert the String
		// to a std::string so we can use the formatted output method.
		// To see why this is necessary, change it to use write() only
		// (unformatted output) and then run simple2: notice that the
		// columnar output formatting is wrecked.
		std::string temp;
		in.to_string(temp);
		o << temp;
	}
	return o;
}



} // end namespace mysqlpp
