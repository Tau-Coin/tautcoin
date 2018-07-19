/***********************************************************************
 test/uds.cpp - Tests the Unix domain socket verifier in
	UnixDomainSocketConnection.  This test always succeeds on Windows!

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

#include <connection.h>
#include <exceptions.h>

#include <iostream>
#include <sstream>
#include <string>

#if !defined(MYSQLPP_PLATFORM_WINDOWS)
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#if !defined(AF_LOCAL)
#	define AF_LOCAL AF_UNIX
#endif

#include <errno.h>
#include <string.h>

static const char* success_path = "test_uds_success.sock";
static const char* failure_path = "test_uds_failure.sock";

static int
make_socket(const char* path, mode_t mode)
{
	// Just in case a socket with this name exists already, try to
	// remove it.  Only a failure if it exists and we can't remove it.
	if ((unlink(path) < 0) && (errno != ENOENT)) {
		return -1;
	}

	// Create the domain socket
	int fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (fd < 0) {
		return -1;
	}
	
	// Bind the socket to the named file
	struct sockaddr_un saun;
	memset(&saun, 0, sizeof(saun));
	saun.sun_family = AF_LOCAL;
	strncpy(saun.sun_path, path, sizeof(saun.sun_path));
	saun.sun_path[sizeof(saun.sun_path) - 1] = '\0';
	if (bind(fd, reinterpret_cast<sockaddr*>(&saun), sizeof(saun)) < 0) {
		return -1;
	}

	// Change the socket's mode as requested
	if (chmod(path, mode) < 0) {
		return -1;
	}

	return fd;
}

	
static void
test_success()
{
	std::string error;
	int fd = make_socket(success_path, S_IREAD | S_IWRITE);
	if (fd >= 0) {
		bool fail = !mysqlpp::UnixDomainSocketConnection::is_socket(
				success_path, &error);
		if (fail) {
			throw mysqlpp::SelfTestFailed(error);
		}
	}
	else {
		std::ostringstream outs;
		outs << "Failed to create test domain socket: " << strerror(errno);
		throw mysqlpp::SelfTestFailed(outs.str());
	}
}


static void
test_failure()
{
	int fd = make_socket(failure_path, S_IREAD);
	if (fd < 0) {
		std::ostringstream outs;
		outs << "Failed to create test domain socket: " << strerror(errno);
		throw mysqlpp::SelfTestFailed(outs.str());
	}

	if (mysqlpp::UnixDomainSocketConnection::is_socket(failure_path)) {
		throw mysqlpp::SelfTestFailed("Failed to fail on read-only socket");
	}
	else if (mysqlpp::UnixDomainSocketConnection::is_socket(
			"BogusBogus.sock")) {
		throw mysqlpp::SelfTestFailed("Failed to fail on bad file name");
	}
	else {
		close(fd);
		unlink(failure_path);
		fd = creat(failure_path, S_IREAD | S_IWRITE);
		bool success = mysqlpp::UnixDomainSocketConnection::is_socket(
				failure_path);
		if (success) {
			throw mysqlpp::SelfTestFailed("Failed to fail on non-socket");
		}
	}
}
#endif


int
main()
{
#if defined(MYSQLPP_PLATFORM_WINDOWS)
	// Test not appropriate to this platform.  Always succeed.
	return 0;
#else
	try {
		test_success();
		unlink(success_path);
		test_failure();
		unlink(failure_path);
		return 0;
	}
	catch (mysqlpp::SelfTestFailed& e) {
		std::cerr << "TCP address parse error: " << e.what() << std::endl;
		return 1;
	}
	catch (std::exception& e) {
		std::cerr << "Unexpected test failure: " << e.what() << std::endl;
		return 2;
	}
#endif
}
