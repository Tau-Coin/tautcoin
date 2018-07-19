/***********************************************************************
 cgi_jpeg.cpp - Example code showing how to fetch JPEG data from a BLOB
 	column and send it back to a browser that requested it by ID.
	
	Use load_jpeg.cpp to load JPEG files into the database we query.

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

#define CRLF			"\r\n"
#define CRLF2			"\r\n\r\n"

int
main(int argc, char* argv[])
{
	// Get database access parameters from command line if present, else
	// use hard-coded values for true CGI case.
	mysqlpp::examples::CommandLine cmdline(argc, argv, "root",
			"nunyabinness");
	if (!cmdline) {
		return 1;
	}

	// Parse CGI query string environment variable to get image ID
	unsigned int img_id = 0;
	char* cgi_query = getenv("QUERY_STRING");
	if (cgi_query) {
		if ((strlen(cgi_query) < 4) || memcmp(cgi_query, "id=", 3)) {
			std::cout << "Content-type: text/plain" << std::endl << std::endl;
			std::cout << "ERROR: Bad query string" << std::endl;
			return 1;
		}
		else {
			img_id = atoi(cgi_query + 3);
		}
	}
	else {
		std::cerr << "Put this program into a web server's cgi-bin "
				"directory, then" << std::endl;
		std::cerr << "invoke it with a URL like this:" << std::endl;
		std::cerr << std::endl;
		std::cerr << "    http://server.name.com/cgi-bin/cgi_jpeg?id=2" <<
				std::endl;
		std::cerr << std::endl;
		std::cerr << "This will retrieve the image with ID 2." << std::endl;
		std::cerr << std::endl;
		std::cerr << "You will probably have to change some of the #defines "
				"at the top of" << std::endl;
		std::cerr << "examples/cgi_jpeg.cpp to allow the lookup to work." <<
				std::endl;
		return 1;
	}

	// Retrieve image from DB by ID
	try {
		mysqlpp::Connection con(mysqlpp::examples::db_name,
				cmdline.server(), cmdline.user(), cmdline.pass());
		mysqlpp::Query query = con.query();
		query << "SELECT * FROM images WHERE id = " << img_id;
		mysqlpp::StoreQueryResult res = query.store();
		if (res && res.num_rows()) {
			images img = res[0];
			if (img.data.is_null) {
				std::cout << "Content-type: text/plain" << CRLF2;
				std::cout << "No image content!" << CRLF;
			}
			else {
				std::cout << "X-Image-Id: " << img_id << CRLF; // for debugging
				std::cout << "Content-type: image/jpeg" << CRLF;
				std::cout << "Content-length: " <<
						img.data.data.length() << CRLF2;
				std::cout << img.data;
			}
		}
		else {
			std::cout << "Content-type: text/plain" << CRLF2;
			std::cout << "ERROR: No image with ID " << img_id << CRLF;
		}
	}
	catch (const mysqlpp::BadQuery& er) {
		// Handle any query errors
		std::cout << "Content-type: text/plain" << CRLF2;
		std::cout << "QUERY ERROR: " << er.what() << CRLF;
		return 1;
	}
	catch (const mysqlpp::Exception& er) {
		// Catch-all for any other MySQL++ exceptions
		std::cout << "Content-type: text/plain" << CRLF2;
		std::cout << "GENERAL ERROR: " << er.what() << CRLF;
		return 1;
	}

	return 0;
}
