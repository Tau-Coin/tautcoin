/***********************************************************************
 test/wnp.cpp - Tests WindowsNamedPipeConnection::is_wnp().  This test
	can only fail on Windows!  It succeeds when built for anything else.

 Copyright (c) 2007 by Educational Technology Resources, Inc.
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

#include <mysql++.h>

#include <iostream>
#include <sstream>


int
main()
{
#if defined(MYSQLPP_PLATFORM_WINDOWS)
	if (!mysqlpp::WindowsNamedPipeConnection::is_wnp(".")) {
		std::cerr << "Failed to identify Windows named pipe" << std::endl;
	
	}
	else if (mysqlpp::WindowsNamedPipeConnection::is_wnp("bogus")) {
		std::cerr << "Failed to fail for bogus named pipe" << std::endl;
	}
	else if (mysqlpp::WindowsNamedPipeConnection::is_wnp(0)) {
		std::cerr << "Failed to fail for null named pipe" << std::endl;
	}
	else {
		return 0;
	}

	return 1;
#else
	return 0;
#endif
}
