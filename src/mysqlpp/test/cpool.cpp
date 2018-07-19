/***********************************************************************
 test/cpool.cpp - Tests the ConnectionPool class.

 Copyright (c) 2007-2008 by Educational Technology Resources, Inc. and
 (c) 2007 by Jonathan Wakely.  Others may also hold copyrights on
 code in this file.  See the CREDITS.txt file in the top directory of
 the distribution for details.

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

#include <cpool.h>
#include <connection.h>

#include <iostream>

#if defined(MYSQLPP_PLATFORM_WINDOWS)
#	define SLEEP(n) Sleep((n) * 1000)
#else
#	include <unistd.h>
#	define SLEEP(n) sleep(n)
#endif

using namespace std;

class TestConnection : public mysqlpp::Connection
{
public:
	TestConnection() : itime_(time(0)) { }
	time_t instantiation_time() const { return itime_; }

private:
	time_t itime_;
};


class TestConnectionPool : public mysqlpp::ConnectionPool
{
public:
	~TestConnectionPool() { clear(); }

	unsigned int max_idle_time() { return 1; }

private:
	TestConnection* create() { return new TestConnection; }
	void destroy(mysqlpp::Connection* cp) { delete cp; }
};


int
main()
{
	TestConnectionPool pool;

	mysqlpp::Connection* conn1 = pool.grab();
	mysqlpp::Connection* conn2 = pool.grab();
	if (conn1 == conn2) {
		cerr << "Pool returned the same connection twice!" << endl;
		return 1;
	}

	pool.release(conn2);
	mysqlpp::Connection* conn3 = pool.grab();
	if (conn2 != conn3) {
		cerr << "conn2 should have been reused but wasn't!" << endl;
		return 1;
	}

	time_t itime_c1 = dynamic_cast<TestConnection*>(conn1)->
			instantiation_time();
	pool.release(conn1);
	SLEEP(pool.max_idle_time() + 1);
	mysqlpp::Connection* conn4 = pool.grab();
	time_t itime_c4 = dynamic_cast<TestConnection*>(conn4)->
			instantiation_time();
	if (itime_c1 == itime_c4) {
		cerr << "conn1 should have been destroyed but wasn't!" << endl;
		return 1;
	}

	pool.release(conn3);
	pool.release(conn4);
	pool.shrink();
	if (!pool.empty()) {
		cerr << "Shrunken pool is not empty!" << endl;
		return 1;
	}

	return 0;
}
