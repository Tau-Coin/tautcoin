/***********************************************************************
 options.cpp - Implements the Option class hierarchy.

 Copyright (c) 2007-2009 by Educational Technology Resources, Inc.
 Others may also hold copyrights on code in this file.  See the
 CREDITS file in the top directory of the distribution for details.

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
#include "options.h"

#include "dbdriver.h"


namespace mysqlpp {

#if !defined(DOXYGEN_IGNORE)
// We're hiding all the Option subclass internals from Doxygen.  All the
// upper-level classes are documented fully, and each leaf class itself
// is documented.  It's just the ctors and set() methods we're refusing
// to document over and over again.

Option::Error
CompressOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_COMPRESS) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
ConnectTimeoutOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_CONNECT_TIMEOUT, &arg_) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
FoundRowsOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(CLIENT_FOUND_ROWS, arg_) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
GuessConnectionOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_GUESS_CONNECTION) ?
				Option::err_NONE : Option::err_api_reject;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
IgnoreSpaceOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(CLIENT_IGNORE_SPACE, arg_) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
InitCommandOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_INIT_COMMAND, arg_.c_str()) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
InteractiveOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(CLIENT_INTERACTIVE, arg_) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
LocalFilesOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(CLIENT_LOCAL_FILES, arg_) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
LocalInfileOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_LOCAL_INFILE, &arg_) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
MultiResultsOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	if (dbd->connected()) {
		return dbd->set_option(arg_ ? MYSQL_OPTION_MULTI_STATEMENTS_ON :
				MYSQL_OPTION_MULTI_STATEMENTS_OFF) ?
				Option::err_NONE : Option::err_api_reject;
	}
	else {
		return dbd->set_option(CLIENT_MULTI_RESULTS, arg_) ?
				Option::err_NONE : Option::err_api_reject;
	}
#else
	return Option::err_api_limit;
#endif
}


Option::Error
MultiStatementsOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	if (dbd->connected()) {
		return dbd->set_option(arg_ ? MYSQL_OPTION_MULTI_STATEMENTS_ON :
				MYSQL_OPTION_MULTI_STATEMENTS_OFF) ?
				Option::err_NONE : Option::err_api_reject;
	}
	else {
		return dbd->set_option(CLIENT_MULTI_STATEMENTS, arg_) ?
				Option::err_NONE : Option::err_api_reject;
	}
#else
	return Option::err_api_limit;
#endif
}


Option::Error
NamedPipeOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_NAMED_PIPE) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
NoSchemaOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(CLIENT_NO_SCHEMA, arg_) ?
				Option::err_NONE : Option::err_api_reject;
}


#if MYSQL_VERSION_ID > 40000		// only in 4.0 +
Option::Error
ProtocolOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_PROTOCOL, &arg_) ?
				Option::err_NONE : Option::err_api_reject;
}
#endif


Option::Error
ReadDefaultFileOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_READ_DEFAULT_FILE, arg_.c_str()) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
ReadDefaultGroupOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_READ_DEFAULT_GROUP, arg_.c_str()) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
ReadTimeoutOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_READ_TIMEOUT, &arg_) ?
				Option::err_NONE : Option::err_api_reject;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
ReconnectOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 50106
	// Option fixed in this version to work correctly whether set before
	// connection comes up, or after
	return dbd->set_option(MYSQL_OPT_RECONNECT, &arg_) ?
			Option::err_NONE : Option::err_api_reject;
#elif MYSQL_VERSION_ID >= 50013
	// Between the time the option was created in 5.0.13 and when it was
	// fixed in 5.1.6, it only worked correctly if set after initial
	// connection.  So, don't accept it if disconnected, even though API
	// does accept it; option gets reset when the connection comes up.
	return dbd->connected() ?
			dbd->set_option(MYSQL_OPT_RECONNECT, &arg_) ?
				Option::err_NONE : Option::err_api_reject :
				Option::err_disconnected;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
ReportDataTruncationOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 50003
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_REPORT_DATA_TRUNCATION, &arg_) ?
				Option::err_NONE : Option::err_api_reject;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
SecureAuthOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_SECURE_AUTH, &arg_) ?
				Option::err_NONE : Option::err_api_reject;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
SetCharsetDirOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_SET_CHARSET_DIR, arg_.c_str()) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
SetCharsetNameOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_SET_CHARSET_NAME, arg_.c_str()) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
SetClientIpOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_SET_CLIENT_IP, arg_.c_str()) ?
				Option::err_NONE : Option::err_api_reject;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
SharedMemoryBaseNameOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40100
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_SHARED_MEMORY_BASE_NAME, arg_.c_str()) ?
				Option::err_NONE : Option::err_api_reject;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
SslOption::set(DBDriver* dbd)
{
#if defined(HAVE_MYSQL_SSL_SET)
	return dbd->connected() ? Option::err_connected :
			dbd->enable_ssl(
				key_.size() ? key_.c_str() : 0,
				cert_.size() ? cert_.c_str() : 0,
				ca_.size() ? ca_.c_str() : 0,
				capath_.size() ? capath_.c_str() : 0,
				cipher_.size() ? cipher_.c_str() : 0) ?
				Option::err_NONE : Option::err_api_reject;
#else
	(void)dbd;
	return Option::err_api_limit;
#endif
}


Option::Error
UseEmbeddedConnectionOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_USE_EMBEDDED_CONNECTION) ?
				Option::err_NONE : Option::err_api_reject;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
UseRemoteConnectionOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_USE_REMOTE_CONNECTION) ?
				Option::err_NONE : Option::err_api_reject;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
WriteTimeoutOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_WRITE_TIMEOUT, &arg_) ?
				Option::err_NONE : Option::err_api_reject;
#else
	return Option::err_api_limit;
#endif
}

#endif // !defined(DOXYGEN_IGNORE)

} // end namespace mysqlpp
