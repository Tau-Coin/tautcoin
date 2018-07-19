/***********************************************************************
 result.cpp - Implements the ResultBase, StoreQueryResult and
	UseQuery Result classes.

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

#include "result.h"

#include "dbdriver.h"


namespace mysqlpp {


ResultBase::ResultBase(MYSQL_RES* res, DBDriver* dbd, bool te) :
OptionalExceptions(te),
driver_(res ? dbd : 0),
fields_(Fields::size_type(res ? dbd->num_fields(res) : 0)),
current_field_(0)
{
	if (res) {
		Fields::size_type i = 0;
		const MYSQL_FIELD* pf;
		while ((i < fields_.size()) && (pf = dbd->fetch_field(res))) {
			fields_[i++] = pf;
		}
		dbd->field_seek(res, 0);		// semantics break otherwise!

		names_ = new FieldNames(this);
		types_ = new FieldTypes(this);
	}
}


ResultBase&
ResultBase::copy(const ResultBase& other)
{
	if (this != &other) {
		set_exceptions(other.throw_exceptions());

		if (other.driver_) {
			driver_ = other.driver_;
			fields_ = other.fields_;
			names_ = other.names_;
			types_ = other.types_;
			current_field_ = other.current_field_;
		}
		else {
			driver_ = 0;
			fields_.clear();
			names_ = 0;
			types_ = 0;
			current_field_ = 0;
		}
	}

	return *this;
}


int
ResultBase::field_num(const std::string& i) const
{
	size_t index = (*names_)[i];
	if ((index >= names_->size()) && throw_exceptions()) {
		if (throw_exceptions()) {
			throw BadFieldName(i.c_str());
		}
		else {
			return -1;
		}
	}
	
	return int(index);
}


StoreQueryResult::StoreQueryResult(MYSQL_RES* res, DBDriver* dbd,
		bool te) :
ResultBase(res, dbd, te),
list_type(list_type::size_type(res && dbd ? dbd->num_rows(res) : 0)),
copacetic_(res && dbd)
{
	if (copacetic_) {
		iterator it = begin();
		while (MYSQL_ROW row = dbd->fetch_row(res)) {
			if (const unsigned long* lengths = dbd->fetch_lengths(res)) {
				*it = Row(row, this, lengths, throw_exceptions());
				++it;
			}
		}

		dbd->free_result(res);
	}
}


StoreQueryResult&
StoreQueryResult::copy(const StoreQueryResult& other)
{
	if (this != &other) {
		ResultBase::copy(other);
		assign(other.begin(), other.end());
		copacetic_ = other.copacetic_;
	}

	return *this;
}


UseQueryResult::UseQueryResult(MYSQL_RES* res, DBDriver* dbd, bool te) :
ResultBase(res, dbd, te)
{
	if (res) {
		result_ = res;
	}
}


UseQueryResult&
UseQueryResult::copy(const UseQueryResult& other)
{
	if (this != &other) {
		ResultBase::copy(other);
		if (other.result_) {
			result_ = other.result_;
		}
		else {
			result_ = 0;
		}
	}

	return *this;
}


const unsigned long*
UseQueryResult::fetch_lengths() const
{
	return driver_->fetch_lengths(result_.raw());
}


Row
UseQueryResult::fetch_row() const
{
	if (!result_) {
		if (throw_exceptions()) {
			throw UseQueryError("Results not fetched");
		}
		else {
			return Row();
		}
	}

	MYSQL_ROW row = driver_->fetch_row(result_.raw());
	if (row) {
		const unsigned long* lengths = fetch_lengths();
		if (lengths) {
			return Row(row, this, lengths, throw_exceptions());
		}
		else {
			if (throw_exceptions()) {
				throw UseQueryError("Failed to get field lengths");
			}
			else {
				return Row();
			}
		}
	}
	else {
		// Prior to v3, this was considered an error, but it just means
		// we've fallen off the end of a "use" query's result set.  You
		// can't predict when this will happen, but it isn't an error.
		// Just return a falsy row object so caller's loop terminates.
		return Row();
	}
}


MYSQL_ROW
UseQueryResult::fetch_raw_row() const
{
	return driver_->fetch_row(result_.raw());
}


} // end namespace mysqlpp

