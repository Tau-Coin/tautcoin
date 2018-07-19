/***********************************************************************
 ssx/main.cpp - Main driver module for ssqlsxlat, which does several
 	data translations related to the SSQLSv2 mechanism of MySQL++.  The
	primary one is SSQLSv2 language files (*.ssqls) to C++ source code,
	but there are others.  Run "ssqlsxlat -?" to get a complete list.

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

#include <cmdline.h>

#include <iostream>

using namespace std;
using namespace mysqlpp::ssqlsxlat;


//// parse_ssqls2 //////////////////////////////////////////////////////
// We were given the name of a putative SSQLS v2 source file; try to
// parse it.

static ParseV2*
parse_ssqls2(const char* file_name)
{
	try {
		cout << "Parsing SSQLS v2 file " << file_name << "..." << endl;
		ParseV2* pt = new ParseV2(file_name);
		cout << file_name << " parsed successfully, " <<
				(pt->end() - pt->begin()) << " interesting lines." <<
				endl;
		return pt;
	}
	catch (const ParseV2::FileException& e) {
		cerr << file_name << ":0" << 
				": file I/O error in SSQLS v2 parse: " <<
				e.what() << endl;
	}
	catch (const ParseV2::ParseException& e) {
		cerr << e.file_name() << ':' << e.line() << ':' <<
				e.what() << endl;
	}
	catch (const std::exception& e) {
		cerr << file_name << ":0" << 
				": critical error in SSQLS v2 parse: " <<
				e.what() << endl;
	}
	return 0;
}


//// main //////////////////////////////////////////////////////////////

int
main(int argc, char* argv[])
{
	// Parse the command line
	CommandLine cmdline(argc, argv);
	if (cmdline) {
		ParseV2* ptree = 0;

		switch (cmdline.input_source()) {
			case CommandLine::ss_ssqls2:
				ptree = parse_ssqls2(cmdline.input());
				break;

			default:
				cerr << "Sorry, I don't yet know what to do with input "
						"source type " << int(cmdline.input_source()) <<
						'!' << endl;
				return 2;
		}

		if (cmdline.output_sink() != CommandLine::ss_unknown) {
			if (ptree) {
				switch (cmdline.output_sink()) {
					case CommandLine::ss_ssqls2:
						if (generate_ssqls2(cmdline.output(), ptree)) {
							return 0;
						}
						else {
							return 2;
						}

					default:
						cerr << "Sorry, I don't yet know what to do "
								"with sink type " <<
								int(cmdline.output_sink()) << '!' <<
								endl;
						return 2;
				}
			}
			else {
				// Depending on someone farther up the line to write
				// the error message, explaining why we didn't get a
				// parse tree.
				return 2;
			}	
		}
		else {
			cerr << "Sorry, I don't know how to write C++ output yet." <<
					endl;
			return 2;
		}
	}
	else {
		return 1;
	}
}

