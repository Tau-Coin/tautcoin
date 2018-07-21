/***********************************************************************
 type_info.cpp - Implements the mysql_type_info class.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB, and
 (c) 2004-2007 by Educational Technology Resources, Inc.  Others may
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

#include "common.h"
#include "type_info.h"

#include "datetime.h"
#include "myset.h"
#include "sql_types.h"

#if defined(MYSQLPP_MYSQL_HEADERS_BURIED)
#	include <mysql/mysql.h>
#else
#	include <mysql.h>
#endif

#include <string>

using namespace std;

namespace mysqlpp {

// This table maps C++ type information to SQL type information.  As you
// can see, it's intimately tied in with MySQL's type constants, thus the
// name.  Unlike in earlier versions of MySQL++, this table is the only
// place with such a dependency.  Everything else abstracts MySQL's
// type system away by bouncing things through this table.
//
// The second half of the table parallels the first, to handle null-able
// versions of the types in the first half.  This is required because
// SQL's 'null' concept does not map neatly into the C++ type system, so
// null-able versions of these types have to have a different C++ type,
// implemented using the Null template.  See null.h for further details.
//
// Types with tf_default set are added to a lookup map in the
// mysql_type_info_lookup class in order to provide reverse lookup
// of C++ types to SQL types.  If you take the subset of all items
// marked as default, the typeid() of each item must be unique.
const mysql_type_info::sql_type_info mysql_type_info::types[] = {
	sql_type_info("DECIMAL NOT NULL", typeid(sql_decimal),
#if MYSQL_VERSION_ID >= 50001
			MYSQL_TYPE_NEWDECIMAL
#else
			MYSQL_TYPE_DECIMAL
#endif
			),
	sql_type_info("TINYINT NOT NULL", typeid(sql_tinyint),
			MYSQL_TYPE_TINY, mysql_ti_sql_type_info::tf_default),
	sql_type_info("TINYINT UNSIGNED NOT NULL", typeid(sql_tinyint_unsigned),
			MYSQL_TYPE_TINY, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("SMALLINT NOT NULL", typeid(sql_smallint),
			MYSQL_TYPE_SHORT, mysql_ti_sql_type_info::tf_default),
	sql_type_info("SMALLINT UNSIGNED NOT NULL", typeid(sql_smallint_unsigned),
			MYSQL_TYPE_SHORT, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("INT NOT NULL", typeid(sql_int),
			MYSQL_TYPE_LONG, mysql_ti_sql_type_info::tf_default),
	sql_type_info("INT UNSIGNED NOT NULL", typeid(sql_int_unsigned),
			MYSQL_TYPE_LONG, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("FLOAT NOT NULL", typeid(sql_float),
			MYSQL_TYPE_FLOAT, mysql_ti_sql_type_info::tf_default),
	sql_type_info("FLOAT UNSIGNED NOT NULL", typeid(sql_float),
			MYSQL_TYPE_FLOAT, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("DOUBLE NOT NULL", typeid(sql_double),
			MYSQL_TYPE_DOUBLE, mysql_ti_sql_type_info::tf_default),
	sql_type_info("DOUBLE UNSIGNED NOT NULL", typeid(sql_double),
			MYSQL_TYPE_DOUBLE, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("NULL NOT NULL", typeid(void),
			MYSQL_TYPE_NULL, mysql_ti_sql_type_info::tf_default),
	sql_type_info("TIMESTAMP NOT NULL", typeid(sql_timestamp),
			MYSQL_TYPE_TIMESTAMP),
	sql_type_info("BIGINT NOT NULL", typeid(sql_bigint),
			MYSQL_TYPE_LONGLONG, mysql_ti_sql_type_info::tf_default),
	sql_type_info("BIGINT UNSIGNED NOT NULL", typeid(sql_bigint_unsigned),
			MYSQL_TYPE_LONGLONG, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("MEDIUMINT NOT NULL", typeid(sql_mediumint),
			MYSQL_TYPE_INT24, mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("MEDIUMINT UNSIGNED NOT NULL", typeid(sql_mediumint_unsigned),
			MYSQL_TYPE_INT24, mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("DATE NOT NULL", typeid(sql_date),
			MYSQL_TYPE_DATE, mysql_ti_sql_type_info::tf_default),
	sql_type_info("TIME NOT NULL", typeid(sql_time),
			MYSQL_TYPE_TIME, mysql_ti_sql_type_info::tf_default),
	sql_type_info("DATETIME NOT NULL", typeid(sql_datetime),
			MYSQL_TYPE_DATETIME, mysql_ti_sql_type_info::tf_default),
	sql_type_info("ENUM NOT NULL", typeid(sql_enum),
			MYSQL_TYPE_ENUM, mysql_ti_sql_type_info::tf_default),
	sql_type_info("SET NOT NULL", typeid(sql_set),
			MYSQL_TYPE_SET, mysql_ti_sql_type_info::tf_default),
	sql_type_info("TINYBLOB NOT NULL", typeid(sql_tinyblob),
			MYSQL_TYPE_TINY_BLOB),
	sql_type_info("MEDIUMBLOB NOT NULL", typeid(sql_mediumblob),
			MYSQL_TYPE_MEDIUM_BLOB),
	sql_type_info("LONGBLOB NOT NULL", typeid(sql_longblob),
			MYSQL_TYPE_LONG_BLOB),
	sql_type_info("BLOB NOT NULL", typeid(sql_blob),
			MYSQL_TYPE_BLOB, mysql_ti_sql_type_info::tf_default),
	sql_type_info("VARCHAR NOT NULL", typeid(sql_varchar),
			MYSQL_TYPE_VAR_STRING, mysql_ti_sql_type_info::tf_default),
	sql_type_info("CHAR NOT NULL", typeid(sql_char),
			MYSQL_TYPE_STRING),

	sql_type_info("DECIMAL NULL", typeid(Null<sql_decimal>),
#if MYSQL_VERSION_ID >= 50001
			MYSQL_TYPE_NEWDECIMAL
#else
			MYSQL_TYPE_DECIMAL
#endif
			, mysql_ti_sql_type_info::tf_null),
	sql_type_info("TINYINT NULL", typeid(Null<sql_tinyint>),
			MYSQL_TYPE_TINY, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("TINYINT UNSIGNED NULL", typeid(Null<sql_tinyint_unsigned>),
			MYSQL_TYPE_TINY, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("SMALLINT NULL", typeid(Null<sql_smallint>),
			MYSQL_TYPE_SHORT, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("SMALLINT UNSIGNED NULL", typeid(Null<sql_smallint_unsigned>),
			MYSQL_TYPE_SHORT, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("INT NULL", typeid(Null<sql_int>),
			MYSQL_TYPE_LONG, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("INT UNSIGNED NULL", typeid(Null<sql_int_unsigned>),
			MYSQL_TYPE_LONG, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("FLOAT NULL", typeid(Null<sql_float>),
			MYSQL_TYPE_FLOAT, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("FLOAT UNSIGNED NULL", typeid(Null<sql_float>),
			MYSQL_TYPE_FLOAT, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("DOUBLE NULL", typeid(Null<sql_double>),
			MYSQL_TYPE_DOUBLE, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("DOUBLE UNSIGNED NULL", typeid(Null<sql_double>),
			MYSQL_TYPE_DOUBLE, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("NULL NULL", typeid(Null<void>),
			MYSQL_TYPE_NULL, mysql_ti_sql_type_info::tf_null),
	sql_type_info("TIMESTAMP NULL", typeid(Null<sql_timestamp>),
			MYSQL_TYPE_TIMESTAMP),
	sql_type_info("BIGINT NULL", typeid(Null<sql_bigint>),
			MYSQL_TYPE_LONGLONG, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("BIGINT UNSIGNED NULL", typeid(Null<sql_bigint_unsigned>),
			MYSQL_TYPE_LONGLONG, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("MEDIUMINT NULL", typeid(Null<sql_mediumint>),
			MYSQL_TYPE_INT24, mysql_ti_sql_type_info::tf_null),
	sql_type_info("MEDIUMINT UNSIGNED NULL", typeid(Null<sql_mediumint_unsigned>), 
			MYSQL_TYPE_INT24, mysql_ti_sql_type_info::tf_null |
			mysql_ti_sql_type_info::tf_unsigned),
	sql_type_info("DATE NULL", typeid(Null<sql_date>),
			MYSQL_TYPE_DATE, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("TIME NULL", typeid(Null<sql_time>),
			MYSQL_TYPE_TIME, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("DATETIME NULL", typeid(Null<sql_datetime>),
			MYSQL_TYPE_DATETIME, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("ENUM NULL", typeid(Null<sql_enum>),
			MYSQL_TYPE_ENUM, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("SET NULL", typeid(Null<sql_set>),
			MYSQL_TYPE_SET, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("TINYBLOB NULL", typeid(Null<sql_tinyblob>),
			MYSQL_TYPE_TINY_BLOB, mysql_ti_sql_type_info::tf_null),
	sql_type_info("MEDIUMBLOB NULL", typeid(Null<sql_mediumblob>),
			MYSQL_TYPE_MEDIUM_BLOB, mysql_ti_sql_type_info::tf_null),
	sql_type_info("LONGBLOB NULL", typeid(Null<sql_longblob>),
			MYSQL_TYPE_LONG_BLOB, mysql_ti_sql_type_info::tf_null),
	sql_type_info("BLOB NULL", typeid(Null<sql_blob>),
			MYSQL_TYPE_BLOB, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("VARCHAR NULL", typeid(Null<sql_varchar>),
			MYSQL_TYPE_VAR_STRING, mysql_ti_sql_type_info::tf_default |
			mysql_ti_sql_type_info::tf_null),
	sql_type_info("CHAR NULL", typeid(Null<sql_char>),
			MYSQL_TYPE_STRING, mysql_ti_sql_type_info::tf_null)
};

const int mysql_type_info::num_types =
		sizeof(mysql_type_info::types) / sizeof(mysql_type_info::types[0]);

const mysql_type_info::sql_type_info_lookup
		mysql_type_info::lookups(mysql_type_info::types,
		mysql_type_info::num_types);

#if !defined(DOXYGEN_IGNORE)
// Doxygen will not generate documentation for this section.

mysql_ti_sql_type_info_lookup::mysql_ti_sql_type_info_lookup(
		const sql_type_info types[], const int size)
{
	for (int i = 0; i < size; ++i) {
		if (types[i].is_default()) {
			map_[types[i].c_type_] = i;
		}
	}
}

#endif // !defined(DOXYGEN_IGNORE)

unsigned char mysql_type_info::type(enum_field_types t,
		bool _unsigned, bool _null)
{
	for (unsigned char i = 0; i < num_types; ++i) {
		if ((types[i].base_type_ == t) &&
				(!_unsigned || types[i].is_unsigned()) &&
				(!_null || types[i].is_null())) {
			return i;
		}
	}

	return type(MYSQL_TYPE_STRING, false, _null);	// punt!
}

bool mysql_type_info::quote_q() const
{
	const type_info& ti = base_type().c_type();
	return ti == typeid(string) ||
			ti == typeid(sql_date) ||
			ti == typeid(sql_time) ||
			ti == typeid(sql_datetime) ||
			ti == typeid(sql_blob) ||
			ti == typeid(sql_tinyblob) ||
			ti == typeid(sql_mediumblob) ||
			ti == typeid(sql_longblob) ||
			ti == typeid(sql_char) ||
			ti == typeid(sql_set);
}

bool mysql_type_info::escape_q() const
{
	const type_info& ti = base_type().c_type();
	return ti == typeid(string) ||
			ti == typeid(sql_enum) ||
			ti == typeid(sql_blob) ||
			ti == typeid(sql_tinyblob) ||
			ti == typeid(sql_mediumblob) ||
			ti == typeid(sql_longblob) ||
			ti == typeid(sql_char) ||
			ti == typeid(sql_varchar);
}

} // end namespace mysqlpp

