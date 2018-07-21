/***********************************************************************
 manip.cpp - Implements MySQL++'s various quoting/escaping stream
	manipulators.

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

#include "manip.h"

#include "query.h"
#include "sqlstream.h"

using namespace std;

namespace mysqlpp {

SQLQueryParms&
operator <<(quote_type2 p, SQLTypeAdapter& in)
{
	if (in.quote_q()) {
		string temp("'", 1), escaped;
		p.qparms->escape_string(&escaped, in.data(), in.length());
		temp.append(escaped);
		temp.append("'", 1);
		*p.qparms << SQLTypeAdapter(temp, true);
		return *p.qparms;
	}
	else {
		in.set_processed();
		return *p.qparms << in;
	}
}


ostream&
operator <<(quote_type1 o, const SQLTypeAdapter& in)
{
	Query* pq = dynamic_cast<Query*>(o.ostr);

	// If it's not a Query*, maybe it's a SQLStream*.
	SQLStream* psqls = pq ? 0 : dynamic_cast<SQLStream*>(o.ostr);

	// If it's a Query or a SQLStream, we'll be using unformatted output.
	if (pq || psqls) {
		if (in.quote_q()) o.ostr->put('\'');

		// Now, is escaping appropriate for source data type of 'in'?
		if (in.escape_q()) {
			string escaped;

			// If it's not a Query*, then it has to be a SQLStream.
			if (pq) {
				pq->escape_string(&escaped, in.data(), in.length());
			}
			else {
				psqls->escape_string(&escaped, in.data(), in.length());
			}

			o.ostr->write(escaped.data(), escaped.length());
		}
		else {
			o.ostr->write(in.data(), in.length());
		}

		if (in.quote_q()) o.ostr->put('\'');
	}
	else {
		// Some other stream type, so use formatted output.  User
		// shouldn't be trying to use the quote manipulator, but
		// that's no reason to break their formatting.
		*o.ostr << string(in.data(), in.length());
	}

	return *o.ostr;
}


ostream&
operator <<(quote_only_type1 o, const SQLTypeAdapter& in)
{
	Query* pq = dynamic_cast<Query*>(o.ostr);

	// If it's not a Query*, maybe it's a SQLStream*.
	SQLStream* psqls = pq ? 0 : dynamic_cast<SQLStream*>(o.ostr);

	// If it's a Query or SQLStream, use unformatted output
	if (pq || psqls) {
		if (in.quote_q()) o.ostr->put('\'');

		o.ostr->write(in.data(), in.length());

		if (in.quote_q()) o.ostr->put('\'');
	}
	else {
		// Some other stream type, so use formatted output.  User
		// shouldn't be trying to use this manipulator on a non-Query
		// stream, but that's no reason to break their formatting.
		*o.ostr << '\'' << in << '\'';
	}

	return *o.ostr;
}


ostream&
operator <<(ostream& o, const SQLTypeAdapter& in)
{
	if (dynamic_cast<Query*>(&o) || dynamic_cast<SQLStream*>(&o)) {
		// It's a Query or a SQLStream, so use unformatted output.
		return o.write(in.data(), in.length());
	}
	else {
		// Some other stream type, so use formatted output.  We do this
		// through the temporary so we remain null-friendly.
		return o << string(in.data(), in.length());
	}
}


SQLQueryParms&
operator <<(quote_only_type2 p, SQLTypeAdapter& in)
{
	if (in.quote_q()) {
		string temp("'", 1);
		temp.append(in.data(), in.length());
		temp.append("'", 1);
		return *p.qparms << SQLTypeAdapter(temp, true);
	}
	else {
		in.set_processed();
		return *p.qparms << in;
	}
}


SQLQueryParms&
operator <<(quote_double_only_type2 p, SQLTypeAdapter& in)
{
	if (in.quote_q()) {
		string temp("\"", 1);
		temp.append(in.data(), in.length());
		temp.append("\"", 1);
		return *p.qparms << SQLTypeAdapter(temp, true);
	}
	else {
		in.set_processed();
		return *p.qparms << in;
	}
}


ostream&
operator <<(quote_double_only_type1 o, const SQLTypeAdapter& in)
{
	Query* pq = dynamic_cast<Query*>(o.ostr);

	// If it's not a Query*, maybe it's a SQLStream*.
	SQLStream* psqls = pq ? 0 : dynamic_cast<SQLStream*>(o.ostr);

	// If it's a Query or a SQLStream, use unformatted output
	if (pq || psqls) {
		if (in.quote_q()) o.ostr->put('"');

		o.ostr->write(in.data(), in.length());
	
		if (in.quote_q()) o.ostr->put('"');
	}
	else {
		// Some other stream type, so use formatted output.  User
		// shouldn't be trying to use this manipulator on a non-Query
		// stream, but that's no reason to break their formatting.
		*o.ostr << '"' << in << '"';
	}

	return *o.ostr;
}


SQLQueryParms&
operator <<(escape_type2 p, SQLTypeAdapter& in)
{
	if (in.escape_q()) {
		string escaped;
		p.qparms->escape_string(&escaped, in.data(), in.length());
		*p.qparms << SQLTypeAdapter(escaped, true);
		return *p.qparms;
	}
	else {
		in.set_processed();
		return *p.qparms << in;
	}
}


ostream&
operator <<(escape_type1 o, const SQLTypeAdapter& in)
{
	Query* pq = dynamic_cast<Query*>(o.ostr);

	// If it's not a Query*, maybe it's a SQLStream*.
	SQLStream* psqls = pq ? 0 : dynamic_cast<SQLStream*>(o.ostr);

	if (pq || psqls) {
		// It's a Query or a SQLStream, so we'll be using unformatted output.
		// Now, is escaping appropriate for source data type of 'in'?
		if (in.escape_q()) {
			string escaped;

			// If it's not a Query*, then it has to be a SQLStream.
			if (pq) {
				pq->escape_string(&escaped, in.data(), in.length());
			}
			else {
				psqls->escape_string(&escaped, in.data(), in.length());
			}

			return o.ostr->write(escaped.data(), escaped.length());
		}
		else {
			// It's not escaped, so just write the unformatted output
			return o.ostr->write(in.data(), in.length());
		}
	}
	else {
		// Some other stream type, so use formatted output.  User
		// shouldn't be trying to use the escape manipulator, but
		// that's no reason to break their formatting.
		return *o.ostr << string(in.data(), in.length());
	}
}


SQLQueryParms&
operator <<(do_nothing_type2 p, SQLTypeAdapter& in)
{
	in.set_processed();
	return *p.qparms << in;
}


ostream&
operator <<(do_nothing_type1 o, const SQLTypeAdapter& in)
{
	if (dynamic_cast<Query*>(o.ostr) || dynamic_cast<SQLStream*>(o.ostr)) {
		// It's a Query or a SQLStream, so use unformatted output
		return o.ostr->write(in.data(), in.length());
	}
	else {
		// Some other stream type, so use formatted output.  User
		// shouldn't be trying to use this manipulator on a non-Query
		// stream, but that's no reason to break their formatting.
		return *o.ostr << in;
	}
}


SQLQueryParms&
operator <<(ignore_type2 p, SQLTypeAdapter& in)
{
	return *p.qparms << in;
}

} // end namespace mysqlpp

