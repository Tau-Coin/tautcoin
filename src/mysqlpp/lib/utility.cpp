/***********************************************************************
 utility.cpp - Implements utility functions used within the library.

 Copyright (c) 2009 by Warren Young.  Others may also hold copyrights
 on code in this file.  See the CREDITS file in the top directory of
 the distribution for details.

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

#include "utility.h"

namespace mysqlpp {
	namespace internal {
		void str_to_lwr(std::string& s)
		{
			std::string::iterator it;
			for (it = s.begin(); it != s.end(); ++it) {
				*it = tolower(*it);
			}
		}

		void str_to_lwr(std::string& ls, const char* mcs)
		{
			ls.reserve(strlen(mcs));
			while (mcs && *mcs) {
				ls += tolower(*mcs++);
			}
		}
	} // end namespace internal
} // end namespace mysqlpp

