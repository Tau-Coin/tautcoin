/***********************************************************************
 datetime.cpp - Implements date and time classes compatible with MySQL's
	various date and time column types.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB, and
 (c) 2004-2008 by Educational Technology Resources, Inc.  Others may
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

#define MYSQLPP_NOT_HEADER
#include "common.h"

#include "datetime.h"
#include "stream2string.h"

#include <iomanip>

#include <stdlib.h>
#include <time.h>

using namespace std;

namespace mysqlpp {

static void
safe_localtime(struct tm* ptm, const time_t t)
{
#if defined(MYSQLPP_HAVE_LOCALTIME_S)
	// common.h detected localtime_s() from native RTL of VC++ 2005 and up
	localtime_s(ptm, &t);
#elif defined(HAVE_LOCALTIME_R)
	// autoconf detected POSIX's localtime_r() on this system
	localtime_r(&t, ptm);
#else
	// No explicitly thread-safe localtime() replacement found.  This
	// may still be thread-safe, as some C libraries take special steps
	// within localtime() to get thread safety, such as TLS.
	memcpy(ptm, localtime(&t), sizeof(tm));
#endif
}


std::ostream& operator <<(std::ostream& os, const Date& d)
{
	char fill = os.fill('0');
	ios::fmtflags flags = os.setf(ios::right);
	os		<< setw(4) << d.year() << '-'
			<< setw(2) << static_cast<int>(d.month()) << '-'
			<< setw(2) << static_cast<int>(d.day());
	os.flags(flags);
	os.fill(fill);
	return os;
}


std::ostream& operator <<(std::ostream& os, const Time& t)
{
	char fill = os.fill('0');
	ios::fmtflags flags = os.setf(ios::right);
	os		<< setw(2) << static_cast<int>(t.hour()) << ':'
			<< setw(2) << static_cast<int>(t.minute()) << ':'
			<< setw(2) << static_cast<int>(t.second());
	os.flags(flags);
	os.fill(fill);
	return os;
}


std::ostream& operator <<(std::ostream& os, const DateTime& dt)
{
	if (dt.is_now()) {
		return os << "NOW()";
	}
	else {
		operator <<(os, Date(dt));
		os << ' ';
		return operator <<(os, Time(dt));
	}
}


Date::Date(time_t t)
{
	struct tm tm;
	safe_localtime(&tm, t);

	year_ = tm.tm_year + 1900;
	month_ = tm.tm_mon + 1;
	day_ = tm.tm_mday;
}


DateTime::DateTime(time_t t)
{
	struct tm tm;
	safe_localtime(&tm, t);

	year_ = tm.tm_year + 1900;
	month_ = tm.tm_mon + 1;
	day_ = tm.tm_mday;
	hour_ = tm.tm_hour;
	minute_ = tm.tm_min;
	second_ = tm.tm_sec;

	now_ = false;
}


Time::Time(time_t t)
{
	struct tm tm;
	safe_localtime(&tm, t);

	hour_ = tm.tm_hour;
	minute_ = tm.tm_min;
	second_ = tm.tm_sec;
}


const char*
Date::convert(const char* str)
{
	char num[5];

	num[0] = *str++;
	num[1] = *str++;
	num[2] = *str++;
	num[3] = *str++;
	num[4] = 0;
	year_ = static_cast<unsigned short>(strtol(num, 0, 10));
	if (*str == '-') str++;

	num[0] = *str++;
	num[1] = *str++;
	num[2] = 0;
	month_ = static_cast<unsigned char>(strtol(num, 0, 10));
	if (*str == '-') str++;

	num[0] = *str++;
	num[1] = *str++;
	num[2] = 0;
	day_ = static_cast<unsigned char>(strtol(num, 0, 10));

	return str;
}


const char*
Time::convert(const char* str)
{
	char num[5];

	num[0] = *str++;
	num[1] = *str++;
	num[2] = 0;
	hour_ = static_cast<unsigned char>(strtol(num,0,10));
	if (*str == ':') str++;

	num[0] = *str++;
	num[1] = *str++;
	num[2] = 0;
	minute_ = static_cast<unsigned char>(strtol(num,0,10));
	if (*str == ':') str++;

	num[0] = *str++;
	num[1] = *str++;
	num[2] = 0;
	second_ = static_cast<unsigned char>(strtol(num,0,10));

	return str;
}


const char*
DateTime::convert(const char* str)
{
	Date d;
	str = d.convert(str);
	year_ = d.year();
	month_ = d.month();
	day_ = d.day();
	
	if (*str == ' ') ++str;

	Time t;
	str = t.convert(str);
	hour_ = t.hour();
	minute_ = t.minute();
	second_ = t.second();

	now_ = false;
	
	return str;
}


int
Date::compare(const Date& other) const
{
	if (year_ != other.year_) return year_ - other.year_;
	if (month_ != other.month_) return month_ - other.month_;
	return day_ - other.day_;
}


int
Time::compare(const Time& other) const
{
	if (hour_ != other.hour_) return hour_ - other.hour_;
	if (minute_ != other.minute_) return minute_ - other.minute_;
	return second_ - other.second_;
}


int
DateTime::compare(const DateTime& other) const
{
	if (now_ && other.now_) {
		return 0;
	}
	else {
		Date d(*this), od(other);
		Time t(*this), ot(other);

		if (int x = d.compare(od)) {
			return x;
		}
		else {
			return t.compare(ot);
		}
	}
}


Date::operator std::string() const
{
	return stream2string(*this);
}


DateTime::operator std::string() const
{
	return stream2string(*this);
}


Time::operator std::string() const
{
	return stream2string(*this);
}


Date::operator time_t() const
{
	struct tm tm;
	safe_localtime(&tm, time(0));

	tm.tm_mday = day_;
	tm.tm_mon = month_ - 1;
	tm.tm_year = year_ - 1900;
	tm.tm_isdst = -1;

	return mktime(&tm);
}


DateTime::operator time_t() const
{
	if (now_) {
		// Many factors combine to make it almost impossible for this
		// case to return the same value as you'd get if you used this
		// in a query.  But, you gotta better idea than to return the
		// current time for an object initialized with the value "now"?
		return time(0);
	}
	else {
		struct tm tm;
		tm.tm_sec = second_;
		tm.tm_min = minute_;
		tm.tm_hour = hour_;
		tm.tm_mday = day_;
		tm.tm_mon = month_ - 1;
		tm.tm_year = year_ - 1900;
		tm.tm_isdst = -1;

		return mktime(&tm);
	}
}


Time::operator time_t() const
{
	struct tm tm;
	safe_localtime(&tm, time(0));

	tm.tm_sec = second_;
	tm.tm_min = minute_;
	tm.tm_hour = hour_;
	tm.tm_isdst = -1;

	return mktime(&tm);
}

} // end namespace mysqlpp

