/***********************************************************************
 printdata.h - Declares utility routines for printing out data in
	common forms, used by most of the example programs.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB, and
 (c) 2004-2009 by Educational Technology Resources, Inc.  Others may
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

#if !defined(MYSQLPP_PRINTDATA_H)
#define MYSQLPP_PRINTDATA_H

#include <mysql++.h>

void print_stock_header(size_t rows);
void print_stock_row(const mysqlpp::Row& r);
void print_stock_row(const mysqlpp::sql_char& item,
		mysqlpp::sql_bigint num, mysqlpp::sql_double weight,
		mysqlpp::sql_decimal_null price, const mysqlpp::sql_date& date);
void print_stock_rows(mysqlpp::StoreQueryResult& res);
void print_stock_table(mysqlpp::Query& query);

#endif // !defined(MYSQLPP_PRINTDATA_H)

