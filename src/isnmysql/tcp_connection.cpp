/***********************************************************************
 tcp_connection.cpp - Implements the TCPConnection class.

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

#define MYSQLPP_NOT_HEADER
#include "common.h"
#include "tcp_connection.h"

#include "exceptions.h"

#if !defined(MYSQLPP_PLATFORM_WINDOWS)
#	include <netdb.h>
#	include <arpa/inet.h>
#endif

#include <ctype.h>
#include <stdlib.h>
#include <climits>

using namespace std;

namespace mysqlpp {


bool
TCPConnection::connect(const char* addr, const char* db,
		const char* user, const char* pass)
{
	error_message_.clear();

	unsigned int port = 0;
	string address;
	if (addr) {
		address = addr;
		if (!parse_address(address, port, error_message_)) {
			return false;
		}
	}

	if (error_message_.empty()) {
		return Connection::connect(db, address.c_str(), user, pass, port);
	}
	else {
		if (throw_exceptions()) {
			throw ConnectionFailed(error_message_.c_str());
		}
		else {
			return false;
		}
	}
}


bool
TCPConnection::parse_address(std::string& addr, unsigned int& port,
		std::string& error)
{
	error.clear();
	
	// Pull off service name or port number, if any
	string service;
	if (addr[0] == '[') {
		// Might be IPv6 address plus port/service in RFC 2732 form.
		string::size_type pos = addr.find(']');
		if ((pos == string::npos) ||
				(addr.find(':', pos + 1) != (pos + 1)) ||
				(addr.find_first_of("[]", pos + 2) != string::npos)) {
			error = "Malformed IPv6 [address]:service combination";
			return false;
		}

		// We can separate address from port/service now
		service = addr.substr(pos + 2);
		addr = addr.substr(1, pos - 1);

		// Ensure that address part is empty or has at least two colons
		if (addr.size() &&
				(((pos = addr.find(':')) == string::npos) ||
				(addr.find(':', pos + 1) == string::npos))) {
			error = "IPv6 literal needs at least two colons";
			return false;
		}
	}
	else {
		// Can only be IPv4 address, so check for 0-1 colons
		string::size_type pos = addr.find(':');
		if (pos != string::npos) {
			if (addr.find(':', pos + 1) != string::npos) {
				error = "IPv4 address:service combo can have only one colon";
				return false;
			}
			
			service = addr.substr(pos + 1);
			addr = addr.substr(0, pos);
		}
	}

	// Turn service into a port number, if it was given.  If not, don't
	// overwrite port because it could have a legal value passed in from
	// Connection.
	if (!service.empty()) {
		if (isdigit(service[0])) {
			port = atoi(service.c_str());
			if ((port < 1) || (port > USHRT_MAX)) {
				error = "Invalid TCP port number " + service;
				return false;
			}
		}
		else {
			servent* pse = getservbyname(service.c_str(), "tcp");
			if (pse) {
				port = ntohs(pse->s_port);
			}
			else {
				error = "Failed to look up TCP service " + service;
				return false;
			}
		}
	}

	// Ensure that there are only alphanumeric characters, dots,
	// dashes and colons in address.  Anything else must be an error.
	for (string::const_iterator it = addr.begin(); it != addr.end(); ++it) {
		string::value_type c = *it;
		if (!(isalnum(c) || (c == '.') || (c == '-') || (c == ':'))) {
			error = "Bad character '";
			error += c;
			error += "' in TCP/IP address";
			return false;
		}
	}

	return true;
}


} // end namespace mysqlpp

