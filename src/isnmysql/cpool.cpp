/***********************************************************************
 cpool.cpp - Implements the ConnectionPool class.

 Copyright (c) 2007-2009 by Educational Technology Resources, Inc. and
 (c) 2007 by Jonathan Wakely.  Others may also hold copyrights on
 code in this file.  See the CREDITS.txt file in the top directory
 of the distribution for details.

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

#include "cpool.h"

#include "connection.h"

#include <algorithm>
#include <functional>

namespace mysqlpp {


/// \brief Functor to test whether a given ConnectionInfo object is
/// "too old".
///
/// \internal This is a template only because ConnectionInfo is private.
/// Making it a template means the private type is only used at the point
/// of instantiation, where it is accessible.

template <typename ConnInfoT>
class TooOld : std::unary_function<ConnInfoT, bool>
{
public:
#if !defined(DOXYGEN_IGNORE)
	TooOld(unsigned int tmax) :
	min_age_(time(0) - tmax)
	{
	}

	bool operator()(const ConnInfoT& conn_info) const
	{
		return !conn_info.in_use && conn_info.last_used <= min_age_;
	}

#endif
private:
	time_t min_age_;
};



//// clear /////////////////////////////////////////////////////////////
// Destroy connections in the pool, either all of them (completely
// draining the pool) or just those not currently in use.  The public
// method shrink() is an alias for clear(false).

void
ConnectionPool::clear(bool all)
{
	ScopedLock lock(mutex_);	// ensure we're not interfered with

	PoolIt it = pool_.begin();
	while (it != pool_.end()) {
		if (all || !it->in_use) {
			remove(it++);
		}
		else {
			++it;
		}
	}
}


//// exchange //////////////////////////////////////////////////////////
// Passed connection is defective, so remove it from the pool and return
// a new one.

Connection*
ConnectionPool::exchange(const Connection* pc)
{
	// Don't grab the mutex first.  remove() and grab() both do.
	// Inefficient, but we'd have to hoist their contents up into this
	// method or extract a mutex-free version of each mechanism for
	// each, both of which are also inefficient.
	remove(pc);
	return grab();
}


//// find_mru //////////////////////////////////////////////////////////
// Find most recently used available connection.  Uses operator< for
// ConnectionInfo to order pool with MRU connection last.  Returns 0 if
// there are no connections not in use.

Connection*
ConnectionPool::find_mru()
{
	PoolIt mru = std::max_element(pool_.begin(), pool_.end());
	if (mru != pool_.end() && !mru->in_use) {
		mru->in_use = true;
		return mru->conn;
	}
	else {
		return 0;
	}
}


//// grab //////////////////////////////////////////////////////////////

Connection*
ConnectionPool::grab()
{
	ScopedLock lock(mutex_);	// ensure we're not interfered with
	remove_old_connections();
	if (Connection* mru = find_mru()) {
		return mru;
	}
	else {
		// No free connections, so create and return a new one.
		pool_.push_back(ConnectionInfo(create()));
		return pool_.back().conn;
	}
}


//// release ///////////////////////////////////////////////////////////

void
ConnectionPool::release(const Connection* pc)
{
	ScopedLock lock(mutex_);	// ensure we're not interfered with

	for (PoolIt it = pool_.begin(); it != pool_.end(); ++it) {
		if (it->conn == pc) {
			it->in_use = false;
			it->last_used = time(0);
			break;
		}
	}
}


//// remove ////////////////////////////////////////////////////////////
// 2 versions:
//
// First takes a Connection pointer, finds it in the pool, and calls
// the second.  It's public, because Connection pointers are all
// outsiders see of the pool.
//
// Second takes an iterator into the pool, destroys the referenced
// connection and removes it from the pool.  This is only a utility
// function for use by other class internals.

void
ConnectionPool::remove(const Connection* pc)
{
	ScopedLock lock(mutex_);	// ensure we're not interfered with

	for (PoolIt it = pool_.begin(); it != pool_.end(); ++it) {
		if (it->conn == pc) {
			remove(it);
			return;
		}
	}
}

void
ConnectionPool::remove(const PoolIt& it)
{
	// Don't grab the mutex.  Only called from other functions that do
	// grab it.
	destroy(it->conn);
	pool_.erase(it);
}


//// remove_old_connections ////////////////////////////////////////////
// Remove connections that were last used too long ago.

void
ConnectionPool::remove_old_connections()
{
	TooOld<ConnectionInfo> too_old(max_idle_time());

	PoolIt it = pool_.begin();
	while ((it = std::find_if(it, pool_.end(), too_old)) != pool_.end()) {
		remove(it++);
	}
}


//// safe_grab /////////////////////////////////////////////////////////

Connection*
ConnectionPool::safe_grab()
{
	Connection* pc;
	while (!(pc = grab())->ping()) {
		remove(pc);
		pc = 0;
	}
	return pc;
}


} // end namespace mysqlpp

