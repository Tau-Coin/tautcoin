/***********************************************************************
 sql_buffer.cpp - Implements the SQLBuffer class.

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

#include "sql_buffer.h"

#include "datetime.h"
#include "sql_types.h"

#include <string.h>

namespace mysqlpp {


SQLBuffer&
SQLBuffer::assign(const char* data, size_type length, mysql_type_info type,
		bool is_null)
{
	replace_buffer(data, length);
	type_ = type;
	is_null_ = is_null;
	return *this;
}

SQLBuffer&
SQLBuffer::assign(const std::string& s, mysql_type_info type, bool is_null)
{
	replace_buffer(s.data(), s.length());
	type_ = type;
	is_null_ = is_null;
	return *this;
}

bool
SQLBuffer::quote_q() const
{
	if ((type_.base_type().c_type() == typeid(mysqlpp::sql_datetime)) &&
			data_ && (length_ >= 5) && (memcmp(data_, "NOW()", 5) == 0)) {
		// The default DATETIME value is special-cased as a call to the
		// SQL NOW() function, which must not be quoted.
		return false;
	}
	else {
		// Normal case: we can infer the need to quote from the type.
		return type_.quote_q();
	}
}

void
SQLBuffer::replace_buffer(const char* pd, size_type length)
{
	delete[] data_;
	data_ = 0;
	length_ = 0;

	if (pd) {
		// The casts for the data member are because the C type system
		// can't distinguish initialization from modification when it
		// happens in 2 steps like this.
		// 
		// We cast away const for pd in case we're on a system that uses
		// the old definition of memcpy() with non-const 2nd parameter.
		data_ = new char[length + 1];
		length_ = length;
		memcpy(const_cast<char*>(data_), const_cast<char*>(pd), length_);
		const_cast<char*>(data_)[length_] = '\0';
	}
}

} // end namespace mysqlpp

