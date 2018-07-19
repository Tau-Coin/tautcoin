/***********************************************************************
 query.cpp - Implements the Query class.

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

#include "query.h"

#include "autoflag.h"
#include "dbdriver.h"
#include "connection.h"

namespace mysqlpp {

// Force insertfrom() policy template instantiation.  Required to make 
// VC++ happy.
Query::RowCountInsertPolicy<Transaction> RowCountInsertPolicyI(0);
Query::SizeThresholdInsertPolicy<Transaction> SizeThresholdInsertPolicyI(0);
Query::MaxPacketInsertPolicy<Transaction> MaxPacketInsertPolicyI(0);

Query::Query(Connection* c, bool te, const char* qstr) :
#if defined(MYSQLPP_HAVE_STD__NOINIT)
// prevents a double-init memory leak in native VC++ RTL (not STLport!)
std::ostream(std::_Noinit),
#else
std::ostream(0),
#endif
OptionalExceptions(te),
template_defaults(this),
conn_(c),
copacetic_(true)
{
	// Set up our internal IOStreams string buffer
	init(&sbuffer_);

	// Insert passed query string into our string buffer, if given
	if (qstr) {
		sbuffer_.str(qstr);
		seekp(0, std::ios::end);	// allow more insertions at end
	} 

	// Override any global locale setting; we want to use the classic C
	// locale so we don't get weird things like thousands separators in
	// integers inserted into the query stream.
	imbue(std::locale::classic());
}

Query::Query(const Query& q) :
#if defined(MYSQLPP_HAVE_STD__NOINIT)
// ditto above
std::ostream(std::_Noinit),
#else
std::ostream(0),
#endif
OptionalExceptions(q.throw_exceptions())
{
	// Set up our internal IOStreams string buffer
	init(&sbuffer_);

	// See above for reason we override locale for Query streams.
	imbue(std::locale::classic());

	// Copy the other query as best we can
	operator =(q);
}


ulonglong
Query::affected_rows()
{
	return conn_->driver()->affected_rows();
}


int
Query::errnum() const
{
	return conn_->errnum();
}


const char*
Query::error() const
{
	return conn_->error();
}


size_t
Query::escape_string(std::string* ps, const char* original,
		size_t length) const
{
	if (conn_ && *conn_) {
		// Normal case
		return conn_->driver()->escape_string(ps, original, length);
	}
	else {
		// Should only happen in test/test_manip.cpp, since it doesn't
		// want to open a DB connection just to test the manipulators.
		return DBDriver::escape_string_no_conn(ps, original, length);
	}
}


size_t
Query::escape_string(char* escaped, const char* original,
		size_t length) const
{
	if (conn_ && *conn_) {
		// Normal case
		return conn_->driver()->escape_string(escaped, original, length);
	}
	else {
		// Should only happen in test/test_manip.cpp, since it doesn't
		// want to open a DB connection just to test the manipulators.
		return DBDriver::escape_string_no_conn(escaped, original, length);
	}
}


bool
Query::exec(const std::string& str)
{
	if ((copacetic_ = conn_->driver()->execute(str.data(),
			static_cast<unsigned long>(str.length()))) == true) {
		if (parse_elems_.size() == 0) {
			// Not a template query, so auto-reset
			reset();
		}
		return true;
	}
	else if (throw_exceptions()) {
		throw BadQuery(error(), errnum());
	}
	else {
		return false;
	}
}


SimpleResult 
Query::execute() 
{ 
	AutoFlag<> af(template_defaults.processing_);
	return execute(str(template_defaults)); 
}


SimpleResult
Query::execute(SQLQueryParms& p)
{
	AutoFlag<> af(template_defaults.processing_);
	return execute(str(p));
}


SimpleResult
Query::execute(const SQLTypeAdapter& s)
{
	if ((parse_elems_.size() == 2) && !template_defaults.processing_) {
		// We're a template query and this isn't a recursive call, so
		// take s to be a lone parameter for the query.  We will come
		// back in here with a completed query, but the processing_
		// flag will be set, allowing us to avoid an infinite loop.
		AutoFlag<> af(template_defaults.processing_);
		return execute(SQLQueryParms() << s);
	}
	else {
		// Take s to be the entire query string
		return execute(s.data(), s.length());
	}
}


SimpleResult
Query::execute(const char* str, size_t len)
{
	if ((parse_elems_.size() == 2) && !template_defaults.processing_) {
		// We're a template query and this isn't a recursive call, so
		// take s to be a lone parameter for the query.  We will come
		// back in here with a completed query, but the processing_
		// flag will be set, allowing us to avoid an infinite loop.
		AutoFlag<> af(template_defaults.processing_);
		return execute(SQLQueryParms() << str << len );
	}
	if ((copacetic_ = conn_->driver()->execute(str, len)) == true) {
		if (parse_elems_.size() == 0) {
			// Not a template query, so auto-reset
			reset();
		}
		return SimpleResult(conn_, insert_id(), affected_rows(), info());
	}
	else if (throw_exceptions()) {
		throw BadQuery(error(), errnum());
	}
	else {
		return SimpleResult();
	}
}


std::string
Query::info()
{
	return conn_->driver()->query_info();
}


ulonglong
Query::insert_id()
{
	return conn_->driver()->insert_id();
}


bool
Query::more_results()
{
	return conn_->driver()->more_results();
}


Query&
Query::operator=(const Query& rhs)
{
	set_exceptions(rhs.throw_exceptions());
	template_defaults = rhs.template_defaults;
	conn_ = rhs.conn_;
	copacetic_ = rhs.copacetic_;

	*this << rhs.sbuffer_.str();

	parse_elems_  = rhs.parse_elems_;
	parsed_names_ = rhs.parsed_names_;
	parsed_nums_  = rhs.parsed_nums_;

	return *this;
}

Query::operator void*() const
{
	return *conn_ && copacetic_ ? const_cast<Query*>(this) : 0;
}


void
Query::parse()
{
	std::string str = "";
	char num[4];
	std::string name;

	char* s = new char[sbuffer_.str().size() + 1];
	memcpy(s, sbuffer_.str().data(), sbuffer_.str().size());
	s[sbuffer_.str().size()] = '\0';
	const char* s0 = s;

	while (*s) {
		if (*s == '%') {
			// Following might be a template parameter declaration...
			s++;
			if (*s == '%') {
				// Doubled percent sign, so insert literal percent sign.
				str += *s++;
			}
			else if (isdigit(*s)) {
				// Number following percent sign, so it signifies a
				// positional parameter.  First step: find position
				// value, up to 3 digits long.
				num[0] = *s;
				s++;
				if (isdigit(*s)) {
					num[1] = *s;
					num[2] = 0;
					s++;
					if (isdigit(*s)) {
						num[2] = *s;
						num[3] = 0;
						s++;
					}
					else {
						num[2] = 0;
					}
				}
				else {
					num[1] = 0;
				}
				signed char n = atoi(num);

				// Look for option character following position value.
				char option = ' ';
				if (*s == 'q' || *s == 'Q') {
					option = *s++;
				}

				// Is it a named parameter?
				if (*s == ':') {
					// Save all alphanumeric and underscore characters
					// following colon as parameter name.
					s++;
					for (/* */; isalnum(*s) || *s == '_'; ++s) {
						name += *s;
					}

					// Eat trailing colon, if it's present.
					if (*s == ':') {
						s++;
					}

					// Update maps that translate parameter name to
					// number and vice versa.
					if (n >= static_cast<short>(parsed_names_.size())) {
						parsed_names_.insert(parsed_names_.end(),
								static_cast<std::vector<std::string>::size_type>(
										n + 1) - parsed_names_.size(),
								std::string());
					}
					parsed_names_[n] = name;
					parsed_nums_[name] = n;
				}

				// Finished parsing parameter; save it.
				parse_elems_.push_back(SQLParseElement(str, option, n));
				str = "";
				name = "";
			}
			else {
				// Insert literal percent sign, because sign didn't
				// precede a valid parameter string; this allows users
				// to play a little fast and loose with the rules,
				// avoiding a double percent sign here.
				str += '%';
			}
		}
		else {
			// Regular character, so just copy it.
			str += *s++;
		}
	}

	parse_elems_.push_back(SQLParseElement(str, ' ', -1));
	delete[] s0;
}


SQLTypeAdapter*
Query::pprepare(char option, SQLTypeAdapter& S, bool replace)
{
	if (S.is_processed()) {
		return &S;
	}

	if (option == 'q') {
		std::string temp(S.quote_q() ? "'" : "", S.quote_q() ? 1 : 0);

		if (S.escape_q()) {
			char *escaped = new char[S.size() * 2 + 1];
			size_t len = conn_->driver()->escape_string(escaped,
					S.data(), static_cast<unsigned long>(S.size()));
			temp.append(escaped, len);
			delete[] escaped;
		}
		else {
			temp.append(S.data(), S.length());
		}

		if (S.quote_q()) temp.append("'", 1);

		SQLTypeAdapter* ss = new SQLTypeAdapter(temp);

		if (replace) {
			S = *ss;
			S.set_processed();
			delete ss;
			return &S;
		}
		else {
			return ss;
		}
	}
	else if (option == 'Q' && S.quote_q()) {
		std::string temp("'", 1);
		temp.append(S.data(), S.length());
		temp.append("'", 1);
		SQLTypeAdapter *ss = new SQLTypeAdapter(temp);

		if (replace) {
			S = *ss;
			S.set_processed();
			delete ss;
			return &S;
		}
		else {
			return ss;
		}
	}
	else {
		if (replace) {
			S.set_processed();
		}
		return &S;
	}
}


void
Query::proc(SQLQueryParms& p)
{
	sbuffer_.str("");

	for (std::vector<SQLParseElement>::iterator i = parse_elems_.begin();
			i != parse_elems_.end(); ++i) {
		MYSQLPP_QUERY_THISPTR << i->before;
		int num = i->num;
		if (num >= 0) {
			SQLQueryParms* c;
			if (size_t(num) < p.size()) {
				c = &p;
			}
			else if (size_t(num) < template_defaults.size()) {
				c = &template_defaults;
			}
			else {
				*this << " ERROR";
				throw BadParamCount(
						"Not enough parameters to fill the template.");
			}

			SQLTypeAdapter& param = (*c)[num];
			if (param.is_null()) {
				MYSQLPP_QUERY_THISPTR << "NULL";
			}
			else {
				SQLTypeAdapter* ss = pprepare(i->option, param, c->bound());
				MYSQLPP_QUERY_THISPTR << *ss;
				if (ss != &param) {
					// pprepare() returned a new string object instead of
					// updating param in place, so we need to delete it.
					delete ss;
				}
			}
		}
	}
}


void
Query::reset()
{
	seekp(0);
	clear();
	sbuffer_.str("");

	parse_elems_.clear();
	template_defaults.clear();
}


bool
Query::result_empty()
{
	return conn_->driver()->result_empty();
}


StoreQueryResult 
Query::store() 
{ 
	AutoFlag<> af(template_defaults.processing_);
	return store(str(template_defaults)); 
}


StoreQueryResult
Query::store(SQLQueryParms& p)
{
	AutoFlag<> af(template_defaults.processing_);
	return store(str(p));
}


StoreQueryResult
Query::store(const SQLTypeAdapter& s)
{
	if ((parse_elems_.size() == 2) && !template_defaults.processing_) {
		// We're a template query and this isn't a recursive call, so
		// take s to be a lone parameter for the query.  We will come
		// back in here with a completed query, but the processing_
		// flag will be set, allowing us to avoid an infinite loop.
		AutoFlag<> af(template_defaults.processing_);
		return store(SQLQueryParms() << s);
	}
	else {
		// Take s to be the entire query string
		return store(s.data(), s.length());
	}
}


StoreQueryResult
Query::store(const char* str, size_t len)
{
	if ((parse_elems_.size() == 2) && !template_defaults.processing_) {
		// We're a template query and this isn't a recursive call, so
		// take s to be a lone parameter for the query.  We will come
		// back in here with a completed query, but the processing_
		// flag will be set, allowing us to avoid an infinite loop.
		AutoFlag<> af(template_defaults.processing_);
		return store(SQLQueryParms() << str << len );
	}
	MYSQL_RES* res = 0;
	if ((copacetic_ = conn_->driver()->execute(str, len)) == true) {
		res = conn_->driver()->store_result();
	}

	if (res) {
		if (parse_elems_.size() == 0) {
			// Not a template query, so auto-reset
			reset();
		}
		return StoreQueryResult(res, conn_->driver(), throw_exceptions());
	}
	else {
		// Either result set is empty, or there was a problem executing
		// the query or storing its results.  Since it's not an error to
		// use store() with queries that never return results (INSERT,
		// DELETE, CREATE, ALTER...) we need to figure out which case
		// this is.  (You might use store() instead of execute() for
		// such queries when the query strings come from "outside".)
		copacetic_ = (conn_->errnum() == 0);
		if (copacetic_) {
			if (parse_elems_.size() == 0) {
				// Not a template query, so auto-reset
				reset();
			}
			return StoreQueryResult();
		}
		else if (throw_exceptions()) {
			throw BadQuery(error(), errnum());
		}
		else {
			return StoreQueryResult();
		}
	}
}


StoreQueryResult
Query::store_next()
{
#if MYSQL_VERSION_ID > 41000		// only in MySQL v4.1 +
	DBDriver::nr_code rc = conn_->driver()->next_result();
	if (rc == DBDriver::nr_more_results) {
		// There are more results, so return next result set.
		MYSQL_RES* res = conn_->driver()->store_result();
		if (res) {
			return StoreQueryResult(res, conn_->driver(),
					throw_exceptions());
		}
		else {
			// Result set is null, but throw an exception only i it is
			// null because of some error.  If not, it's just an empty
			// result set, which is harmless.  We return an empty result
			// set if exceptions are disabled, as well.
			if (conn_->errnum() && throw_exceptions()) {
				throw BadQuery(error(), errnum());
			}
			else {
				return StoreQueryResult();
			}
		}
	}
	else if (throw_exceptions()) {
		if (rc == DBDriver::nr_error) {
			throw BadQuery(error(), errnum());
		}
		else if (conn_->errnum()) {
			throw BadQuery(error(), errnum());
		}
		else {
			return StoreQueryResult();	// normal end-of-result-sets case
		}
	}
	else {
		return StoreQueryResult();
	}
#else
	return store();
#endif // MySQL v4.1+
}


std::string
Query::str(SQLQueryParms& p)
{
	if (!parse_elems_.empty()) {
		proc(p);
	}

	return sbuffer_.str();
}


UseQueryResult 
Query::use() 
{ 
	AutoFlag<> af(template_defaults.processing_);
	return use(str(template_defaults)); 
}


UseQueryResult
Query::use(SQLQueryParms& p)
{
	AutoFlag<> af(template_defaults.processing_);
	return use(str(p));
}


UseQueryResult
Query::use(const SQLTypeAdapter& s)
{
	if ((parse_elems_.size() == 2) && !template_defaults.processing_) {
		// We're a template query and this isn't a recursive call, so
		// take s to be a lone parameter for the query.  We will come
		// back in here with a completed query, but the processing_
		// flag will be set, allowing us to avoid an infinite loop.
		AutoFlag<> af(template_defaults.processing_);
		return use(SQLQueryParms() << s);
	}
	else {
		// Take s to be the entire query string
		return use(s.data(), s.length());
	}
}


UseQueryResult
Query::use(const char* str, size_t len)
{
	if ((parse_elems_.size() == 2) && !template_defaults.processing_) {
		// We're a template query and this isn't a recursive call, so
		// take s to be a lone parameter for the query.  We will come
		// back in here with a completed query, but the processing_
		// flag will be set, allowing us to avoid an infinite loop.
		AutoFlag<> af(template_defaults.processing_);
		return use(SQLQueryParms() << str << len );
	}
	MYSQL_RES* res = 0;
	if ((copacetic_ = conn_->driver()->execute(str, len)) == true) {
		res = conn_->driver()->use_result();
	}

	if (res) {
		if (parse_elems_.size() == 0) {
			// Not a template query, so auto-reset
			reset();
		}
		return UseQueryResult(res, conn_->driver(), throw_exceptions());
	}
	else {
		// See comments in store() above for why we distinguish between
		// empty result sets and actual error returns here.
		copacetic_ = (conn_->errnum() == 0);
		if (copacetic_) {
			if (parse_elems_.size() == 0) {
				// Not a template query, so auto-reset
				reset();
			}
			return UseQueryResult();
		}
		else if (throw_exceptions()) {
			throw BadQuery(error(), errnum());
		}
		else {
			return UseQueryResult();
		}
	}
}


} // end namespace mysqlpp

