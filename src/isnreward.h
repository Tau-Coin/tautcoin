// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ISNCOIN_DATABASE_CREATE_H
#define ISNCOIN_DATABASE_CREATE_H

#include <mysql++.h>
#include <ssqls.h>

//clubinfo
sql_create_3(clubinfo,
	1, 3,
	mysqlpp::sql_int_unsigned_null, club_id,
	mysqlpp::sql_varchar, address,
	mysqlpp::sql_int_unsigned, ttc)

//memberinfo
sql_create_6(memberinfo,
	1, 6,
	mysqlpp::sql_int_unsigned_null, address_id,
	mysqlpp::sql_varchar, address,
	mysqlpp::sql_int_unsigned, club_id,
	mysqlpp::sql_int_unsigned, father,
	mysqlpp::sql_int_unsigned, tc,
	mysqlpp::sql_float, balance)

#endif // ISNCOIN_DATABASE_CREATE_H
