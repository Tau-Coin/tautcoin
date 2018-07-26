/***********************************************************************
 ssqls1.cpp - Example that produces the same results as simple1, but it
	uses a Specialized SQL Structure to store the results instead of a
	MySQL++ Result object.
 
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


#include <iostream>
#include <vector>

#include "isndb.cpp"

using namespace std;

int
main(int argc, char *argv[])
{

	ISNDB* imtest = ISNDB::GetInstance();
	string tablename, condition, cvalue;
	vector<string> field, values;

	//select test
	tablename= "clubinfo";
	field.push_back("ttc");
	condition= "address";
	cvalue= "imtestadd";
	mysqlpp::StoreQueryResult dataSelect= imtest->ISNSqlSelectAA(tablename, field, condition, cvalue);
	cout << "select test is: " << dataSelect[0]["ttc"] << endl;
	field.clear();

	//update test
	tablename="memberinfo";
	field.push_back("father");
	values.push_back("24");
	condition= "imaddtc";
	mysqlpp::SimpleResult  dataUpdate= imtest->ISNSqlUpdate(tablename, field, values, condition);
	cout << "update test is: " << dataUpdate << endl;
	field.clear();
	
	//add one test
	tablename="memberinfo";
	field.push_back("tc");
	condition= "imaddtc";
	mysqlpp::SimpleResult  dataAddOne= imtest->ISNSqlAddOne(tablename, field, condition);
	cout << "add one test is: " << dataAddOne << endl;
	field.clear();
	values.clear();
	
	//insert test
	tablename="memberinfo";
	values.push_back("imadd1");
	values.push_back("3");
	values.push_back("11");
	values.push_back("9");
	values.push_back("19");
	int  dataInsert= imtest->ISNSqlInsert(tablename, values);
	cout << "insert id is: " << dataInsert << endl;

	return 0;
}
