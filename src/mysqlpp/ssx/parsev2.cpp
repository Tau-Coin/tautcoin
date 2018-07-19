/***********************************************************************
 ssx/parsev2.cpp - Parser for the SSQLS v2 language.

 Copyright (c) 2009 by Warren Young.  Others may also hold copyrights
 on code in this file.  See the CREDITS.txt file in the top directory
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

#include "parsev2.h"

#include <utility.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <iostream>

using namespace std;

// System error value to string conversion
#if defined(MYSQLPP_PLATFORM_WINDOWS)
#	define SYSERR ""
#else
#	define SYSERR ": " << strerror(errno)
#endif

////////////////////////////////////////////////////////////////////////
// Instantiate static members

ParseV2::StringList ParseV2::File::search_path_;


ParseV2::ParseV2(const char* file_name) :
file_(file_name)
{
	// For each line in the file, read it in and try to make sense of it
	// based on the indent level and the leading verb.
	//cout << "TRACE: parsing SSQLS v2 file " << file_name << '.' << endl;
	string line;
	bool subdirective;
	while (file_.read_line(line, subdirective)) {
		// Skip empty lines
		if (line.empty()) continue;

		// Break line up into whitespace-separated tokens
		StringList tokens;
		tokenize(tokens, line);
		if (tokens.empty()) continue;		// shouldn't happen

		// Try to interpret token list
		Line* line = Line::parse(tokens, subdirective, file_);
		assert(line != 0);	// errors should be signalled with exceptions
		Include* iline = dynamic_cast<Include*>(line);
		if (iline) {
			// Include lines are a special case: we hoist the parsed
			// lines from the included file up into our line list.
			lines_.reserve(lines_.size() + (iline->end() - iline->begin()));
			for (LineListIt it = iline->begin(); it != iline->end(); ++it) {
				lines_.push_back(*it);
			}
			delete iline;
		}
		else {
			// Normal case: add line to our line list
			lines_.push_back(line);
		}
	}
}


void
ParseV2::tokenize(StringList& tokens, const std::string& line) const
{
	// Skip over leading whitespace
	string::const_iterator current = line.begin();
	while (current != line.end() && isspace(*current)) {
		++current;
	}

	// Break rest of line up into space-separated words, treating
	// consecutive spaces as single separators, and ignoring trailing
	// spaces.
	while (current != line.end()) {
		string::const_iterator word_start = current;
		while (current != line.end() && !isspace(*current)) ++current;
		tokens.push_back(string(word_start, current));
		while (current != line.end() && isspace(*current)) ++current;
	}
#if 0
	cout << "TRACE: " << tokens.size() << " tokens in line '" << line <<
			"':" << endl;
	cout << "\t";
	for (StringListIt it = tokens.begin(); it != tokens.end(); /* */) {
		cout << '\'' << *it++ << '\'';
		if (it != tokens.end()) cout << ',';
	}
	cout << endl;
#endif
}


void
ParseV2::AccessorStyleOption::print(std::ostream& os) const
{
	os << "option accessor_style ";
	switch (type_) {
		case camel_case_lower:	os << "getX"; break;
		case camel_case_upper:	os << "GetX"; break;
		case overloaded:		os << "x"; break;
		case stroustrup:		os << "get_x"; break;
		case unknown:			os << "UNKNOWN"; break;
	}
}


ParseV2::AccessorStyleOption::Type
ParseV2::AccessorStyleOption::parse_type(const std::string& style,
		const File& file)
{
	//cout << "TRACE: found accessor style " << style << endl;
	if (style.compare("getX") == 0) {
		return camel_case_lower;
	}
	else if (style.compare("GetX") == 0) {
		return camel_case_upper;
	}
	else if (style.compare("get_x") == 0) {
		return stroustrup;
	}
	else if (style.compare("x") == 0) {
		return overloaded;
	}
	else {
		ostringstream o;
		o << "unknown accessor style '" << style << '\'';
		file.parse_error(o);
		return unknown;
	}
}


void
ParseV2::ExceptionOnSchemaMismatchOption::print(std::ostream& os) const
{
	os << "option exception_on_schema_mismatch " <<
			(throw_ ? "true" : "false");
}


ParseV2::Field*
ParseV2::Field::parse(const StringList& tl, bool subdirective,
		const File& file)
{
	// Check for obviously wrong inputs
	if (tl.size() < 2) {
		file.parse_error("'field' directive requires at least one argument");
	}
	else if (!subdirective) {
		file.parse_error("'field' must be a subdirective");
	}

	// Get field name
	StringListIt it = ++tl.begin();	// skip over 'field'
	string name(*it++);
	//cout << "TRACE: found field " << name << endl;

	// Scan rest of token list, interpreting the name/value pairs.
	string type, alias;
	bool is_autoinc = false, is_key = false, is_null = false,
			is_unsigned = false;
	while (tl.end() - it > 1) {
		string attr(*it++), value(*it++);
		mysqlpp::internal::str_to_lwr(attr);
		if (attr.compare("alias") == 0) {
			alias = value;
		}
		else if (attr.compare("is") == 0) {
			if (value.compare("autoinc") == 0) {
				is_autoinc = true;
			}
			else if (value.compare("key") == 0) {
				is_key = true;
			}
			else if (value.compare("null") == 0) {
				is_null = true;
			}
			else if (value.compare("unsigned") == 0) {
				is_unsigned = true;
			}
			else {
				ostringstream o;
				o << "unknown is-attribute '" << value << 
						"' for field '" << name << '\'';
				file.parse_error(o);
			}
		}
		else if (attr.compare("type") == 0) {
			type = value;
		}
		else {
			ostringstream o;
			o << "bad attribute '" << attr << "' for field " << name;
			file.parse_error(o);
		}
	}

	// Warn if field wasn't given an explicit type
	if (type.empty()) {
		cerr << "Warning: field '" << name << "' defaulting to "
				"string type." << endl;
	}

	// No attribute errors, so create the Field object.  
	return new Field(name, type, is_unsigned, is_null, is_autoinc,
			is_key, alias);
}


void
ParseV2::Field::print(std::ostream& os) const
{
	os << "field " << name_ << ' ';
	type_.print(os);
	if (is_autoinc_)	os << " is autoinc";
	if (is_key_)		os << " is key";
	if (is_null_)		os << " is null";
	if (is_unsigned_)	os << " is unsigned";
	if (alias_.size())	os << " alias " << alias_;
}


ParseV2::Field::Type::Type(const std::string& s) :
value_(ft_string)
{
	// Force s to lowercase as ls, for easier comparisons below
	string ls(s);
	mysqlpp::internal::str_to_lwr(ls);
	//cout << "TRACE: field type " << s << endl;

	// Suss out appropriate field type in piecewise fashion.  This
	// parser isn't terribly robust, but it should do sane things in the
	// presence of plausible input.  Feed it garbage, and the worst that
	// will happen is that it will either give up, letting the default
	// of ft_string stand, or it may happen to recognize some substring
	// in the input that causes it to assign some other type.
	if ((ls.find("blob") != string::npos) ||
			(ls.find("varbinary") != string::npos)) {
		value_ = ft_blob;
	}
	else if (ls.find("bool") == 0) {
		value_ = ft_tinyint;
	}
	else if (ls.find("date") == 0) {
		if (ls.compare("datetime") == 0) value_ = ft_datetime;
		else value_ = ft_date;
	}
	else if ((ls.find("decimal") != string::npos) ||
			(ls.find("double") != string::npos) ||
			(ls.find("fixed") != string::npos) ||
			(ls.find("numeric") != string::npos)) {
		value_ = ft_double;
	}
	else if (ls.find("float") != string::npos) {
		value_ = ft_float;
	}
	else if (ls.find("int") != string::npos) {
		if (ls.find("tiny") == 0) value_ = ft_tinyint;
		else if (ls.compare("int1") == 0) value_ = ft_tinyint;
		else if (ls.find("small") == 0) value_ = ft_smallint;
		else if (ls.compare("int2") == 0) value_ = ft_smallint;
		else if (ls.find("big") == 0) value_ = ft_bigint;
		else if (ls.compare("int8") == 0) value_ = ft_bigint;
		else value_ = ft_mediumint;
	}
	else if (ls.find("set") != string::npos) {
		value_ = ft_set;
	}
	else if (ls.find("time") == 0) {
		if (ls.compare("timestamp") == 0) value_ = ft_datetime;
		else value_ = ft_time;
	}
}


void
ParseV2::Field::Type::print(std::ostream& os) const
{
	os << "type ";
	switch (value_) {
		case ft_bigint:		os << "bigint";		break;
		case ft_blob:		os << "blob";		break;
		case ft_date:		os << "date";		break;
		case ft_datetime:	os << "datetime";	break;
		case ft_double:		os << "double";		break;
		case ft_float:		os << "float";		break;
		case ft_mediumint:	os << "mediumint";	break;
		case ft_set:		os << "set";		break;
		case ft_smallint:	os << "smallint";	break;
		case ft_string:		os << "string";		break;
		case ft_time:		os << "time";		break;
		case ft_tinyint:	os << "tinyint";	break;
	}
}


ParseV2::File::File(const char* file_name) :
file_name_(file_name),
line_number_(1)
{
	// Try to open the named file for reading
	//cout << "TRACE: opening file " << file_name << endl;
	StringListIt it = search_path_.begin();
	string path(file_name_);
	while (true) {
		ifs_.open(path.c_str());
		if (ifs_) {
			// Opened it, so add directory part of file path (if any) to
			// our search path.
			add_directory_to_search_path(file_name);
			break;
		}
		else if (it == search_path_.end()) {
			// Ran out of things to try
			ostringstream o;
			o << "Failed to open '" << file_name_ << "' for reading" <<
					SYSERR;
			error(o);
		}
		else {
			// Replace previous path with the next possibility, which
			// we'll try on the next iteration.
			path = *it++;
			path += MYSQLPP_PATH_SEPARATOR;
			path += file_name_;
		}
	}
}


void
ParseV2::File::add_directory_to_search_path(const char* filepath)
{
	//cout << "TRACE: adding directory part of " << filepath << 
	//		" to search path." << endl;
	StringList parts;
	split_path(parts, filepath);
	if (parts.size() > 1) {
		// There's a path part, so check that it's not just '.', the
		// current directory on all the systems we're portable to.
		if (parts.size() != 2 || parts[0].compare(".") != 0) {
			// Path is interesting, so reassemble the directory part
			string path(parts[0]);
			StringListIt it = ++parts.begin();
			while (it < parts.end() - 1) {	// ignore last part; file name
				path += MYSQLPP_PATH_SEPARATOR;
				path += *it++;
			}

			// Add that directory to the search path unless it's already
			// in the list.
			if (find(search_path_.begin(), search_path_.end(), path) ==
					search_path_.end()) {
				search_path_.push_back(path);
				//cout << "TRACE: added new directory " << path << 
				//		" to search path." << endl;
			}
		}
	}
}


void
ParseV2::File::error(const std::string& msg) const
{
	throw FileException(msg);
}


void
ParseV2::File::parse_error(const std::string& msg) const
{
	throw ParseException(msg, file_name_, line_number_);
}


bool
ParseV2::File::read_line(std::string& line, bool& subdirective)
{
	line.clear();
	if (ifs_) {
		static char temp[100];
		ifs_.getline(temp, sizeof(temp));
		if (ifs_) {
			++line_number_;
			line = temp;
			size_t n = line.find('#');
			if (n != string::npos) {
				line.resize(n);
			}
			if (line.size()) {
				subdirective = isspace(line[0]);
			}
			return true;
		}
	}
	return false;
}


void
ParseV2::File::split_path(StringList& parts, const std::string& path) const
{
	//cout << "TRACE: splitting path '" << path << "'..." << endl;
	const char sep = MYSQLPP_PATH_SEPARATOR;
	for (string::const_iterator it = path.begin(); it < path.end(); /* */) {
		string::const_iterator part_start = it;
		while (it != path.end() && (*it != sep)) ++it;
		parts.push_back(string(part_start, it));
		while (it != path.end() && (*it == sep)) ++it;
	}
#if 0
	cout << "TRACE: " << parts.size() << " parts in path '" << path << "': ";
	for (StringListIt it = parts.begin(); it != parts.end(); /* */) {
		cout << *it++;
		if (it != parts.end()) cout << sep;
	}
	cout << endl;
#endif
}


void
ParseV2::HeaderExtensionOption::print(std::ostream& os) const
{
	os << "option header_extension " << value();
}


void
ParseV2::ImplementationExtensionOption::print(std::ostream& os) const
{
	os << "option implementation_extension " << value();
}


ParseV2::Include*
ParseV2::Include::parse(const StringList& tl, bool subdirective,
		const File& file)
{
	// Check for obviously wrong inputs
	if (tl.size() != 2) {
		file.parse_error("'include' directive requires one argument");
	}
	else if (subdirective) {
		file.parse_error("'include' cannot be a subdirective");
	}

	// Above ensures that there is only one argument, so create Include
	// object from it, assuming it's a file name.
	//cout << "TRACE: including " << tl[0] << "..." << endl;
	return new Include((++tl.begin())->c_str());
}


ParseV2::Line*
ParseV2::Line::parse(const StringList& tl, bool subdirective,
		const File& file)
{
	// True if the last top-level directive we saw was 'table'
	static bool last_tld_was_table = false;

	// Pull the directive off the front of the list
	assert(!tl.empty());
	string directive(tl[0]);

	// First, recognize directives that expect subdirectives.
	mysqlpp::internal::str_to_lwr(directive);
	//cout << "TRACE: found " << directive << " line." << endl;
	if (directive.compare("table") == 0) {
		if (!subdirective) last_tld_was_table = true;
		return ParseV2::Table::parse(tl, subdirective, file);
	}
	else {
		// Next recognize directives that either stand alone at the top
		// level, or are always subdirectives.
		if (!subdirective) {
			last_tld_was_table = false;
		}

		if (directive.compare("field") == 0) {
			if (last_tld_was_table) {
				return ParseV2::Field::parse(tl, subdirective, file);
			}
			else {
				file.parse_error("'field' directive must follow 'table'");
			}
		}
		else if (directive.compare("include") == 0) {
			return ParseV2::Include::parse(tl, subdirective, file);
		}
		else if (directive.compare("option") == 0) {
			return ParseV2::Option::parse(tl, subdirective, file);
		}
	}

	// None of the code above recognized the directive.  Yell about it
	// with an exception.  Return doesn't actually happen, it just
	// squishes a compiler warning.
	ostringstream o;
	o << "unrecognized directive '" << directive << '\'';
	file.parse_error(o);
	return 0;
}


ParseV2::Option*
ParseV2::Option::parse(const StringList& tl, bool subdirective,
		const File& file)
{
	// Check for obviously wrong inputs
	if (tl.size() != 3) {
		file.parse_error("'option' directive requires two arguments");
	}
	else if (subdirective) {
		file.parse_error("'option' cannot be a subdirective");
	}

	// See if we can make sense of the first argument, which should be
	// the option name.
	string name(tl[1]);
	string value(tl[2]);
	//cout << "TRACE: found " << name << " = '" << value <<
	//		"' option." << endl;
	if (name.compare("accessor_style") == 0) {
		return new AccessorStyleOption(value, file);
	}
	else if (name.compare("exception_on_schema_mismatch") == 0) {
		return new ExceptionOnSchemaMismatchOption(value);
	}
	else if (name.compare("header_extension") == 0) {
		return new HeaderExtensionOption(value);
	}
	else if (name.compare("implementation_extension") == 0) {
		return new ImplementationExtensionOption(value);
	}
	else {
		ostringstream o;
		o << "unknown option '" << name << '\'';
		file.parse_error(o);
		return 0;
	}
}


bool
ParseV2::Option::parse_bool(const std::string& value)
{
	// Lowercase the given string for easier comparison
	string b(value);
	mysqlpp::internal::str_to_lwr(b);
	
	// Recognize only known falsy values, as SSQLS v2 options all
	// default to false, so the assumption is that any option that's
	// set at all is almost certainly to flip it from its default, to
	// true.  So, only definite false values are actually critical to
	// recognize, that being the surprising case.
	return !(
			b.compare("0") == 0 ||
			b.compare("false") == 0 ||
			b.compare("no") == 0 ||
			b.compare("off") == 0
		);
}


ParseV2::Table::Table(const std::string& name, const std::string& alias,
		const std::string& filebase) :
name_(name),
alias_(alias.empty() ? name : alias),
filebase_(filebase.empty() ? name : filebase)
{
}


ParseV2::Table*
ParseV2::Table::parse(const StringList& tl, bool subdirective,
		const File& file)
{
	// Check for obviously wrong inputs
	if (tl.size() < 2) {
		file.parse_error("'table' directive requires at least one argument");
	}
	else if (subdirective) {
		file.parse_error("'table' cannot be a subdirective");
	}

	// Get table name
	StringListIt it = ++tl.begin();	// skip over 'table'
	string name(*it++);
	//cout << "TRACE: found '" << name << "' table." << endl;

	// Scan rest of token list, interpreting the name/value pairs.
	string alias, filebase;
	while (tl.end() - it > 1) {
		string attr(*it++), value(*it++);
		mysqlpp::internal::str_to_lwr(attr);
		if (attr.compare("alias") == 0) {
			alias = value;
		}
		else if (attr.compare("filebase") == 0) {
			filebase = value;
		}
		else {
			ostringstream o;
			o << "bad attribute '" << attr << "' for table " << name;
			file.parse_error(o);
		}
	}

	// No attribute errors, so create the Table object.  
	return new Table(name, alias, filebase);
}


void
ParseV2::Table::print(std::ostream& os) const
{
	os << "table " << name_;
	if (alias_.size())		os << " alias " << alias_;
	if (filebase_.size())	os << " filebase " << filebase_;
}


ostream&
operator<<(ostream& os, const ParseV2::Line& line)
{
	line.print(os);
	return os;
}

