/***********************************************************************
 vallist.cpp - Implements utility functions for building value lists.
	This is internal functionality used within the library.

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

#include "vallist.h"

#include "result.h"
#include "row.h"

using std::string;

namespace mysqlpp {

void
create_vector(size_t size, std::vector<bool>& v, bool t0, bool t1, bool t2,
		bool t3, bool t4, bool t5, bool t6, bool t7, bool t8, bool t9,
		bool ta, bool tb, bool tc)
{
	v.reserve(size);

	v.push_back(t0);
	if (size == 1) return;

	v.push_back(t1);
	if (size == 2) return;

	v.push_back(t2);
	if (size == 3) return;

	v.push_back(t3);
	if (size == 4) return;

	v.push_back(t4);
	if (size == 5) return;

	v.push_back(t5);
	if (size == 6) return;

	v.push_back(t6);
	if (size == 7) return;

	v.push_back(t7);
	if (size == 8) return;

	v.push_back(t8);
	if (size == 9) return;

	v.push_back(t9);
	if (size == 10) return;

	v.push_back(ta);
	if (size == 11) return;

	v.push_back(tb);
	if (size == 12) return;

	v.push_back(tc);
}


template <class Container>
void create_vector(const Container& c, std::vector<bool>& v,
		std::string s0, std::string s1, std::string s2, std::string s3,
		std::string s4, std::string s5, std::string s6, std::string s7,
		std::string s8, std::string s9, std::string sa, std::string sb,
		std::string sc)
{
	v.insert(v.begin(), c.size(), false);

	v[c.field_num(s0.c_str())] = true;
	if (s1.empty()) return;

	v[c.field_num(s1.c_str())] = true;
	if (s2.empty()) return;

	v[c.field_num(s2.c_str())] = true;
	if (s3.empty()) return;

	v[c.field_num(s3.c_str())] = true;
	if (s4.empty()) return;

	v[c.field_num(s4.c_str())] = true;
	if (s5.empty()) return;

	v[c.field_num(s5.c_str())] = true;
	if (s6.empty()) return;

	v[c.field_num(s6.c_str())] = true;
	if (s7.empty()) return;

	v[c.field_num(s7.c_str())] = true;
	if (s8.empty()) return;

	v[c.field_num(s8.c_str())] = true;
	if (s9.empty()) return;

	v[c.field_num(s9.c_str())] = true;
	if (sa.empty()) return;

	v[c.field_num(sa.c_str())] = true;
	if (sb.empty()) return;

	v[c.field_num(sb.c_str())] = true;
	if (sc.empty()) return;

	v[c.field_num(sc.c_str())] = true;
}


#if !defined(DOXYGEN_IGNORE)
// Instantiate above template.  Not sure why this is necessary.  Hide it
// from Doxygen, because we clearly cannot appease it by documenting it.
template void
create_vector(const Row& c, std::vector<bool>& v, string s0,
		string s1, string s2, string s3, string s4, string s5,
		string s6, string s7, string s8, string s9, string sa,
		string sb, string sc);
#endif

} // end namespace mysqlpp

