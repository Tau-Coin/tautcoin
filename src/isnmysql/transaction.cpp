/***********************************************************************
 transaction.cpp - Implements the Transaction class.

 Copyright © 2006-2014 by Educational Technology Resources, Inc.
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

#include "transaction.h"

#include "connection.h"
#include "query.h"

using namespace std;
using namespace mysqlpp;


//// ctors /////////////////////////////////////////////////////////////

Transaction::Transaction(Connection& conn, bool consistent) :
conn_(conn),
finished_(true)		// don't bother rolling it back if ctor fails
{
	// Begin the transaction set
	Query q(conn_.query("START TRANSACTION"));
	if (consistent) {
		q << " WITH CONSISTENT SNAPSHOT";
	}
	q.execute();

	// Setup succeeded, so mark our transaction as not-finished.
	finished_ = false;
}

Transaction::Transaction(Connection& conn, IsolationLevel level,
		IsolationScope scope, bool consistent) :
conn_(conn),
finished_(true)		// don't bother rolling it back if ctor fails
{
	// Set the transaction isolation level and scope as the user wishes
	Query q(conn_.query("SET "));
	if (scope == session) q << "SESSION ";
	if (scope == global)  q << "GLOBAL ";
	q << "TRANSACTION ISOLATION LEVEL ";
	switch (level) {
		case read_uncommitted:	q << "READ UNCOMMITTED"; break;
		case read_committed:	q << "READ COMMITTED";   break;
		case repeatable_read:	q << "REPEATABLE READ";  break;
		case serializable:		q << "SERIALIZABLE";     break;
	}
	q.execute();

	// Begin the transaction set.  Note that the above isn't part of
	// the transaction, on purpose, so that scope == transaction affects
	// *this* transaction, not the next one.
	q << "START TRANSACTION";
	if (consistent) {
		q << " WITH CONSISTENT SNAPSHOT";
	}
	q.execute();

	// Setup succeeded, so mark our transaction as not-finished.
	finished_ = false;
}


//// dtor //////////////////////////////////////////////////////////////

Transaction::~Transaction()
{
	if (!finished_) {
		try {
			rollback();
		}
		catch (...) {
			// eat all exceptions
		}
	}
}


//// commit ////////////////////////////////////////////////////////////

void
Transaction::commit()
{
	conn_.query("COMMIT").execute();
	finished_ = true;
}


//// rollback //////////////////////////////////////////////////////////

void
Transaction::rollback()
{
	conn_.query("ROLLBACK").execute();
	finished_ = true;
}


