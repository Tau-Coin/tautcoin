/***********************************************************************
 ssqls2.cpp - Implements the SsqlsBase class.

 Copyright (c) 2009 by Educational Technology Resources, Inc.
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
#include "ssqls2.h"

#include "connection.h"
#include "exceptions.h"
#include "query.h"

using namespace std;

// All of the Active Record calls below follow a common pattern, which
// we can only express as a macro.  They just wrap similar calls in
// Query, varying only in minor per-call specific details.
#define QUERY_ACTIVE_RECORD_WRAPPER(action, conn, fs) \
	if (conn) conn_ = conn; \
	if (conn_) { \
		if (conn_->connected()) { \
			if (populated(fs)) return conn_->query().action(*this).execute(); \
			else if (conn_->throw_exceptions()) throw BadQuery( \
					"Cannot " #action " insufficiently populated SSQLS"); \
			else return false; \
		} \
		else if (conn_->throw_exceptions()) throw ConnectionFailed( \
				"Cannot " #action " SSQLS without established connection"); \
		else return false; \
	} \
	else throw ObjectNotInitialized(typeid(*this).name());

namespace mysqlpp {

bool
SsqlsBase::create(Connection* conn) const
{
	(void)conn;
	//TODO define Query::insert(SsqlsBase&)
	//QUERY_ACTIVE_RECORD_WRAPPER(insert, conn, fs_not_autoinc);
	return false;
}

bool
SsqlsBase::load(Connection* conn) const
{
	(void)conn;
	//TODO define Query::select(SsqlsBase&)
	//QUERY_ACTIVE_RECORD_WRAPPER(select, conn, fs_key);
	return false;
}

bool
SsqlsBase::remove(Connection* conn) const
{
	(void)conn;
	//TODO define Query::remove(SsqlsBase&)
	//QUERY_ACTIVE_RECORD_WRAPPER(remove, conn, fs_key);
	return false;
}

bool
SsqlsBase::save(Connection* conn) const
{
	(void)conn;
	//TODO define Query::update(SsqlsBase&)
	//QUERY_ACTIVE_RECORD_WRAPPER(update, conn, fs_all);
	return false;
}

} // end namespace mysqlpp

