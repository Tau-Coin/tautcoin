/***********************************************************************
 qparms.cpp - Implements the SQLQueryParms class.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999, 2000 and 2001 by
 MySQL AB, and (c) 2004-2007 by Educational Technology Resources, Inc.
 Others may also hold copyrights on code in this file.  See the CREDITS
 file in the top directory of the distribution for details.

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

#include "qparms.h"

#include "query.h"

using namespace std;

namespace mysqlpp {

size_t
SQLQueryParms::escape_string(std::string* ps, const char* original,
		size_t length) const
{
	return parent_ ? parent_->escape_string(ps, original, length) : 0;
}

size_t
SQLQueryParms::escape_string(char* escaped, const char* original,
		size_t length) const
{
	return parent_ ? parent_->escape_string(escaped, original, length) : 0;
}

SQLTypeAdapter&
SQLQueryParms::operator [](const char* str)
{
	if (parent_) {
		return operator [](parent_->parsed_nums_[str]);
	}
	throw ObjectNotInitialized("SQLQueryParms object has no parent!");
}

const SQLTypeAdapter&
SQLQueryParms::operator[] (const char* str) const
{
	if (parent_) {
		return operator [](parent_->parsed_nums_[str]);
	}
	throw ObjectNotInitialized("SQLQueryParms object has no parent!");
}

SQLQueryParms
SQLQueryParms::operator +(const SQLQueryParms& other) const
{
	if (other.size() <= size()) {
		return *this;
	}
	SQLQueryParms New = *this;
	size_t i;
	for (i = size(); i < other.size(); i++) {
		New.push_back(other[i]);
	}

	return New;
}


} // end namespace mysqlpp
