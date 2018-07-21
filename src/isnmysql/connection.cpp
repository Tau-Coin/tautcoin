/***********************************************************************
 connection.cpp - Implements the Connection class.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB, and
 (c) 2004-2008 by Educational Technology Resources, Inc.  Others may
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
#include "connection.h"

#include "dbdriver.h"
#include "query.h"
#include "result.h"

using namespace std;

namespace mysqlpp {

Connection::Connection(bool te) :
OptionalExceptions(te),
driver_(new DBDriver()),
copacetic_(true)
{
}


Connection::Connection(const char* db, const char* server,
		const char* user, const char* password, unsigned int port) :
OptionalExceptions(),
driver_(new DBDriver()),
copacetic_(true)
{
	try {
		connect(db, server, user, password, port);
	}
	catch (...) {
		// Exception thrown from ctor causes dtor to not be called, so
		// we have to delete the DBDriver instance, else we'll leak it.
		delete driver_;
		throw;
	}
}


Connection::Connection(const Connection& other) :
OptionalExceptions(other.throw_exceptions()),
driver_(new DBDriver(*other.driver_))
{
	copy(other);
}


Connection::~Connection()
{
	disconnect();
	delete driver_;
}


void
Connection::build_error_message(const char* core)
{
	error_message_ = "Can't ";
	error_message_ += core;
	error_message_ += " while disconnected";
}


std::string
Connection::client_version() const
{
	return driver_->client_version();
}


bool
Connection::connect(const char* db, const char* server,
		const char* user, const char* password, unsigned int port)
{
	// Figure out what the server parameter means, then try to establish
	// the connection.
	error_message_.clear();
	string host, socket_name;
	copacetic_ = parse_ipc_method(server, host, port, socket_name) &&
			driver_->connect(host.c_str(),
			(socket_name.empty() ? 0 : socket_name.c_str()), port, db,
			user, password);

	// If it failed, decide how to tell the user
	if (!copacetic_ && throw_exceptions()) {
		throw ConnectionFailed(error(), errnum());
	}
	else {
		return copacetic_;
	}
}


bool
Connection::connected() const
{
	return driver_->connected();
}


void
Connection::copy(const Connection& other)
{
	error_message_.clear();
	set_exceptions(other.throw_exceptions());
	driver_->copy(*other.driver_);
}


ulonglong
Connection::count_rows(const std::string& table)
{
	error_message_.clear();
	Query q(this, throw_exceptions());
	q << "SELECT COUNT(*) FROM `" << table << '`';
	if (StoreQueryResult res = q.store()) {
		return res[0][0];
	}
	else {
		return 0;
	}
}


bool
Connection::create_db(const std::string& db)
{
	error_message_.clear();
	Query q(this, throw_exceptions());
	q << "CREATE DATABASE `" << db << '`';
	return q.exec();
}


void
Connection::disconnect()
{
	error_message_.clear();
	driver_->disconnect();
}


bool
Connection::drop_db(const std::string& db)
{
	error_message_.clear();
	Query q(this, throw_exceptions());
	q << "DROP DATABASE `" << db << '`';
	return q.exec();
}


int
Connection::errnum()
{
	return driver_->errnum();
}


const char*
Connection::error() const
{
	return error_message_.size() ? error_message_.c_str() : driver_->error();
}


std::string
Connection::ipc_info() const
{
	return driver_->ipc_info();
}


bool
Connection::kill(unsigned long tid) const
{
	error_message_.clear();
	return driver_->kill(tid);
}


Connection&
Connection::operator=(const Connection& rhs)
{
	copy(rhs);
	return *this;
}


bool
Connection::parse_ipc_method(const char* server, std::string& host,
		unsigned int& port, std::string& socket_name)
{
	// NOTE: This routine has no connection type knowledge.  It can only
	// recognize a 0 value for the server parameter.  All substantial
	// tests are delegated to our specialized subclasses, which figure
	// out what kind of connection the server address denotes.  We do
	// the platform-specific tests first as they're the most reliable.
	
	if (server == 0) {
		// Just take all the defaults
		return true;
	}
	else if (WindowsNamedPipeConnection::is_wnp(server)) {
		// Use Windows named pipes
		host = server;
		return true;
	}
	else if (UnixDomainSocketConnection::is_socket(server)) {
		// Use Unix domain sockets
		socket_name = server;
		return true;
	}
	else {
		// Failing above, it can only be some kind of TCP/IP address.
		host = server;
		return TCPConnection::parse_address(host, port, error_message_);
	}
}


bool
Connection::ping()
{
	if (connected()) {
		error_message_.clear();
		return driver_->ping();
	}
	else {
		// Not connected, and we've forgotten everything we need in
		// order to re-connect, if we once were connected.
		build_error_message("ping database server");
		return false;
	}
}


int
Connection::protocol_version() const
{
	return driver_->protocol_version();
}


Query
Connection::query(const char* qstr)
{
	return Query(this, throw_exceptions(), qstr);
}


Query
Connection::query(const std::string& qstr)
{
	return query(qstr.c_str());
}


bool
Connection::select_db(const std::string& db)
{
	error_message_.clear();
	if (connected()) {
		if (driver_->select_db(db.c_str())) {
			return true;
		}
		else {
			if (throw_exceptions()) {
				throw DBSelectionFailed(error(), errnum());
			}
			return false;
		}
	}
	else {
		build_error_message("select a database");
		if (throw_exceptions()) {
			throw DBSelectionFailed(error_message_.c_str());
		}
		return false;
	}
}


std::string
Connection::server_status() const
{
	return driver_->server_status();
}


std::string
Connection::server_version() const
{
	return driver_->server_version();
}


bool
Connection::set_option(Option* o)
{
	const std::type_info& oti = typeid(*o);
	if (driver_->set_option(o)) {
		error_message_.clear();
		return true;
	}
	else {
		error_message_ = driver_->error();
		if (throw_exceptions()) {
			throw BadOption(error_message_, oti);
		}
		return false;
	}
}


bool
Connection::shutdown()
{
	error_message_.clear();
	if (connected()) {
		if (driver_->shutdown()) {
			return true;
		}
		else {
			if (throw_exceptions()) {
				throw ConnectionFailed(error(), errnum());
			}
			return false;
		}
	}
	else {
		build_error_message("shutdown database server");
		if (throw_exceptions()) {
			throw ConnectionFailed(error_message_.c_str());
		}
		return false;
	}
}


bool
Connection::thread_aware()
{
	return DBDriver::thread_aware();
}


void
Connection::thread_end()
{
	DBDriver::thread_end();
}


unsigned long
Connection::thread_id()
{
	return driver_->thread_id();
}


bool
Connection::thread_start()
{
	return DBDriver::thread_start();
}

} // end namespace mysqlpp

