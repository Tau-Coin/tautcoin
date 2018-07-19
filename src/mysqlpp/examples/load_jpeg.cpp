/***********************************************************************
 load_jpeg.cpp - Example showing how to insert BLOB data into the
	database from a file.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB, and
 (c) 2004-2009 by Educational Technology Resources, Inc.  Others may
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

#include "cmdline.h"
#include "images.h"
#include "printdata.h"

#include <fstream>

using namespace std;
using namespace mysqlpp;


// This is just an implementation detail for the example.  Skip down to
// main() for the concept this example is trying to demonstrate.  You
// can simply assume that, given a BLOB containing a valid JPEG, it
// returns true.
static bool
is_jpeg(const mysqlpp::sql_blob& img, const char** whynot)
{
	// See http://stackoverflow.com/questions/2253404/ for
	// justification for the various tests.
	const unsigned char* idp =
			reinterpret_cast<const unsigned char*>(img.data());
	if (img.size() < 125) {
		*whynot = "a valid JPEG must be at least 125 bytes";
	}
	else if ((idp[0] != 0xFF) || (idp[1] != 0xD8)) {
		*whynot = "file does not begin with JPEG sigil bytes";
	}
	else if ((memcmp(idp + 6, "JFIF", 4) != 0) &&
			 (memcmp(idp + 6, "Exif", 4) != 0)) {
		*whynot = "file does not contain JPEG type word";
	}
	else {
		*whynot = 0;
		return true;
	}

	return false;
}


// Skip to main() before studying this.  This is a little too
// low-level to bother with on your first pass thru the code.
static bool
load_jpeg_file(const mysqlpp::examples::CommandLine& cmdline,
		images& img, string& img_name)
{
	if (cmdline.extra_args().size() == 0) {
		// Nothing for us to do here.  Caller will insert NULL BLOB.
		return true;
	}

	// Got a file's name on the command line, so open it.
	img_name = cmdline.extra_args()[0];
	ifstream img_file(img_name.c_str(), ios::binary);
	if (img_file) {
		// Slurp file contents into RAM with minimum copying.  (Idiom
		// explained here: http://stackoverflow.com/questions/116038/)
		//
		// By loading the file into a C++ string (stringstream::str())
		// and assigning that directly to a mysqlpp::sql_blob, we avoid
		// truncating the binary data at the first null character.
		img.data.data = static_cast<const stringstream*>(
				&(stringstream() << img_file.rdbuf()))->str();

		// Check JPEG data for sanity.
		const char* error;
		if (is_jpeg(img.data.data, &error)) {
			return true;
		}
		else {
			cerr << '"' << img_name << "\" isn't a JPEG: " <<
					error << '!' << endl;
		}
	}

	cmdline.print_usage("[jpeg_file]");
	return false;
}


int
main(int argc, char *argv[])
{
	// Get database access parameters from command line
	mysqlpp::examples::CommandLine cmdline(argc, argv);
	if (!cmdline) {
		return 1;
	}

	try {
		// Establish the connection to the database server.
		mysqlpp::Connection con(mysqlpp::examples::db_name,
				cmdline.server(), cmdline.user(), cmdline.pass());

		// Load the file named on the command line
		images img(mysqlpp::null, mysqlpp::null);
		string img_name("NULL");
		if (load_jpeg_file(cmdline, img, img_name)) {
			// Insert image data or SQL NULL into the images.data BLOB
			// column.  The key here is that we're holding the raw
			// binary data in a mysqlpp::sql_blob, which avoids data
			// conversion problems that can lead to treating BLOB data
			// as C strings, thus causing null-truncation.  The fact
			// that we're using SSQLS here is a side issue, simply
			// demonstrating that mysqlpp::Null<mysqlpp::sql_blob> is
			// now legal in SSQLS, as of MySQL++ 3.0.7.
			Query query = con.query();
			query.insert(img);
			SimpleResult res = query.execute();

			// Report successful insertion
			cout << "Inserted \"" << img_name <<
					"\" into images table, " << img.data.data.size() <<
					" bytes, ID " << res.insert_id() << endl;
		}
	}
	catch (const BadQuery& er) {
		// Handle any query errors
		cerr << "Query error: " << er.what() << endl;
		return -1;
	}
	catch (const BadConversion& er) {
		// Handle bad conversions
		cerr << "Conversion error: " << er.what() << endl <<
				"\tretrieved data size: " << er.retrieved <<
				", actual size: " << er.actual_size << endl;
		return -1;
	}
	catch (const Exception& er) {
		// Catch-all for any other MySQL++ exceptions
		cerr << "Error: " << er.what() << endl;
		return -1;
	}

	return 0;
}
