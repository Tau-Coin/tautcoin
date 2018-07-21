/***********************************************************************
 field_names.cpp - Implements the FieldNames class.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB, and
 (c) 2004-2010 by Educational Technology Resources, Inc.  Others may
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

#define MYSQLPP_NOT_HEADER
#include "common.h"

#include "field_names.h"
#include "result.h"

#include <algorithm>

namespace mysqlpp {

namespace internal { extern void str_to_lwr(std::string& s); }

void
FieldNames::init(const ResultBase* res)
{
	size_t num = res->num_fields();
	reserve(num);

	for (size_t i = 0; i < num; i++) {
		push_back(res->fields().at(i).name());
	}
}


unsigned int
FieldNames::operator [](const std::string& s) const
{
	std::string temp1(s);
	internal::str_to_lwr(temp1);
	for (const_iterator it = begin(); it != end(); ++it) {
	std::string temp2(*it);
		internal::str_to_lwr(temp2);
		if (temp2.compare(temp1) == 0) {
			return it - begin();
		}
	}

	return end() - begin();
}

} // end namespace mysqlpp
