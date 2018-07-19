/***********************************************************************
 ssx/genv2.cpp - Walks the SSQLS v2 parse result, writing back out
 	the equivalent SSQLS v2 DSL code.  This is useful for testing that
	our parser has correctly understood a given piece of code.  It is
	also something like the preprocessor mode of a C++ compiler,
	emitting a digested version of its input.

 Copyright (c) 2009 by Warren Young and (c) 2009-2010 by Educational
 Technology Resources, Inc.  Others may also hold copyrights on code
 in this file.  See the CREDITS.txt file in the top directory of the
 distribution for details.

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

#include "genv2.h"

#include "parsev2.h"

#include <cstring>
#include <iostream>
#include <fstream>
#include <typeinfo>

using namespace std;

//// generate_ssqls2 ///////////////////////////////////////////////////
// 2 versions: second checks its arguments and opens the named file,
// calling the second to actually generate the SSQLS v2 output from
// the parse result only if we were given sane parameters.

static bool
generate_ssqls2(ostream& os, const ParseV2* pparse)
{
	ParseV2::LineListIt it;
	for (it = pparse->begin(); it != pparse->end(); ++it) {
		if (dynamic_cast<ParseV2::Field*>(*it)) {
			// 'field' directives must be indented under the preceding
			// 'table'.  We don't want to hard-code this in
			// ParseV2::Field::print() in case we later start calling
			// those routines for other reasons, such as to construct
			// error messages.  It's really a special case of -o, not
			// really something that print() routine should know.
			os << '\t';
		}
		os << **it << endl;
	}

	return true;
}

bool
generate_ssqls2(const char* file_name, const ParseV2* pparse)
{
	if (pparse) {
		if (strcmp(file_name, "-") == 0) {
			return generate_ssqls2(cout, pparse);
		}
		else {
			ofstream ofs(file_name);
			if (ofs) {
				cout << "TRACE: Generating SSQLS v2 file " << file_name <<
						" from " << (pparse->end() - pparse->begin()) <<
						" line parse result." << endl;
				return generate_ssqls2(ofs, pparse);
			}
			else {
				cerr << "Failed to open " << file_name << \
						" for writing for -o!" << endl;
				return false;
			}
		}
	}
	else {
		cerr << "No parse result given to -o handler!" << endl;
		return false;
	}
}
