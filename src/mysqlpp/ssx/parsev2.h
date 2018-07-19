/***********************************************************************
 ssx/parsev2.h - Declares the SSQLS v2 language parsing related classes.

 Copyright (c) 2009 by Warren Young and (c) 2009-2010 by Educational
 Technology Resources, Inc.  Others may also hold copyrights on code
 in this file.  See the CREDITS.txt file in the top directory of the
 distribution for details.

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

#if !defined(MYSQLPP_SSX_PARSEV2_H)
#define MYSQLPP_SSX_PARSEV2_H

#include <exceptions.h>

#include <cassert>
#include <fstream>
#include <string>
#include <vector>

/// \brief Parses SSQLS v2 documents and holds the parse result
///
/// Construct an object of this type from a file name, and you will
/// get either a parse result or a thrown exception.
class ParseV2
{
public:
	/// \brief List of tokens as returned by boost::algorithm::split
	typedef std::vector<std::string> StringList;

	/// \brief Iterator into a StringList
	///
	/// This is const because StringLists are never modified once created
	typedef StringList::const_iterator StringListIt;

	/// \brief Holds information about an SSQLS v2 file we're parsing
	///
	/// This class exists simply because there's so much file-related
	/// processing in the parser, and it's confusing to have it
	/// scattered about.
	class File
	{
	public:
		/// \brief Open a file for reading, using the search path to
		/// file the file if the direct path isn't readable.
		File(const char* file_name);

		/// \brief Throw a FileException containing the given message
		void error(const std::string& msg) const;

		/// \sa error(const std::string&)
		void error(const std::ostringstream& msg) const
				{ error(msg.str()); }

		/// \brief Return the file's name
		const char* name() const { return file_name_.c_str(); }

		/// \brief Throw a ParseException containing the given message
		/// and our stored info about the file name and current line
		void parse_error(const std::string& msg) const;

		/// \sa parse_error(const std::string&)
		void parse_error(const std::ostringstream& msg) const
				{ error(msg.str()); }

		/// \brief Read a line in from a file
		///
		/// We read the line from our internal file object, trimming any
		/// trailing comment.  Does \e not trim whitespace; we depend on
		/// ParseV2::tokenize() to cope with that.  Sets subdirective
		/// flag if we see leading whitespace before we trim it away, as
		/// indented lines are interpreted differently by File's users.
		///
		/// \return false if our internal file object is in an error
		/// condition on entering or exiting this function.
		bool read_line(std::string& line, bool& subdirective);

	private:
		/// \brief Given the path to a file that is known to exist,
		/// extract the directory part (if any) and apppend it to our
		/// file search path.
		///
		/// This allows 'include' directives to find other files in the
		/// same directory as the included file when the first inclusion
		/// refers to a file not in the current directory.  Because the
		/// search path is static, it doesn't matter who "learns" about
		/// a useful directory.  Every file open from there on benefits.
		void add_directory_to_search_path(const char* filepath);

		/// \brief Break a file path up into a series of elements by the
		/// platform's directory separator, dropping '.' elements.
		void split_path(StringList& parts, const std::string& path) const;

		/// \brief File we're reading from
		std::ifstream ifs_;

		/// \brief Name of we're parsing
		///
		/// Used to construct useful error messages
		std::string file_name_;

		/// \brief Line number in file we're parsing
		///
		/// Used to construct useful error messages
		size_t line_number_;

		/// \brief Directories used in searching for included files
		///
		/// This is a \c vector, rather than a \c set, because order
		/// matters.  We grow this list as we go deeper into an inclusion
		/// tree, with the earlier paths taking precedence.
		///
		/// It's static because we only need on search path, and want
		/// new additions to inform all future parses in this session.
		static StringList search_path_;
	};

	//// Types of parsed lines, and related stuff
	/// \brief Base class for parsed SSQLS v2 declaration lines
	class Line
	{
	public:
		/// \brief Virtual dtor, since this is a base class
		virtual ~Line() { }

		/// \brief Virtual ctor, creating one of our subclass objects
		/// based on what we're passed
		///
		/// \param tl list of tokens found on a line in an SSQLS v2 file
		/// \param subdirective true if there was leading whitespace on
		///        that line, which changes how we interpret tl
		/// \param file information about the file we're currently parsing
		///
		/// \return Line object pointer if line was successfully parsed,
		/// else 0
		static Line* parse(const StringList& tl, bool subdirective,
				const File& file);

		/// \brief Print line's contents out to a stream in SSQLS v2
		/// form.
		virtual void print(std::ostream& os) const = 0;

	protected:
		/// \brief Protected ctor, to prevent instantiation
		Line() { }
	};

	/// \brief A list of pointers to Line objects
	///
	/// These are used for holding the results of the file parsing step
	typedef std::vector<Line*> LineList;

	/// \brief Iterator into a LineList
	///
	/// It's const because once the parse is completed, we switch into
	/// LineList traversal mode, which doesn't modify the list.
	typedef LineList::const_iterator LineListIt;

	/// \brief 'field' directive line
	class Field : public Line
	{
	public:
		/// \brief Holds information about a SQL field declared in
		/// the SSQLS v2 language
		///
		/// \param type the field's SQL type
		/// \param is_unsigned true if type is an integer and is unsigned
		/// \param is_null true if field's value is nullable
		/// \param is_autoinc true if DB automatically assigns an
		///        auto-incrementing value to this field in INSERT if
		///        it isn't specified
		/// \param is_key true if field is part of the primary key
		/// \param name the field's SQL name
		/// \param alias the field's C++ name, defaulting to the SQL
		///        name
		Field(const std::string& name, const std::string& type,
				bool is_unsigned = false, bool is_null = false,
				bool is_autoinc = false, bool is_key = false,
				const std::string& alias = 0) :
		name_(name),
		type_(type),
		is_autoinc_(is_autoinc),
		is_key_(is_key),
		is_null_(is_null),
		is_unsigned_(is_unsigned),
		alias_(alias)
		{
		}

		/// \brief Attempt to create a Field object from information in
		/// the passed StringList
		///
		/// A kind of pre-processor for the Field ctor, creating one of
		/// those objects only if the given StringList makes sense,
		/// using the values we find in that StringList as parameters to
		/// the ctor.
		static Field* parse(const StringList& tl, bool subdirective,
				const File& file);

		/// \brief Print field description out to a stream in SSQLS v2
		/// form.
		void print(std::ostream& os) const;

		/// \brief A smart enum for converting SQL type strings to one
		/// of a relatively few types we directly support.
		///
		/// This object defaults to a value of ft_string if the type
		/// cannot be discerned.
		///
		/// This class is public only to allow it to be tested, in
		/// test/ssqls2.cpp.  It shouldn't actually be used outside
		/// ssqlsxlat.
		class Type
		{
		public:
			/// \brief Known SQL field types
			///
			/// This list is shorter than what we support in MySQL++'s
			/// lib/sql_types.h and shorter still than the full list of
			/// types that SQL database engines support.  Its length is
			/// limited by the diversity of data types in C++ and
			/// MySQL++.  We map SQL types to one of these values as
			/// best we can.
			///
			/// This list doesn't encode anything about nullness,
			/// signedness, etc.  Those are considered attributes modifying
			/// the type, not creating independent data types.  We store
			/// these flags in separate variables in the outer class.
			enum Value {
				ft_tinyint,		///< TINYINT, INT1, BOOL
				ft_smallint,	///< SMALLINT, INT2
				ft_mediumint,	///< INT, MEDIUMINT, INT3, INT4
				ft_bigint,		///< BIGINT, INT8
				ft_float,		///< FLOAT, FLOAT4, FLOAT8
				ft_double,		///< DOUBLE, DECIMAL, FIXED, NUMERIC
				ft_string,		///< *CHAR, ENUM, *TEXT
				ft_blob,		///< *BLOB, VARBINARY
				ft_date,		///< DATE
				ft_datetime,	///< DATETIME, TIMESTAMP
				ft_time,		///< TIME
				ft_set			///< SET
			};

			/// \brief Constructor
			///
			/// Given a SQL type string, try to figure out which of the
			/// relatively small set of known values to use.  Defaults
			/// to ft_string if we can't find a more appropriate type,
			/// as all SQL values can be dealt with as strings.
			Type(const std::string& s);

			/// \brief Copy constructor
			Type(Value v) :
			value_(v)
			{
			}

			/// \brief Print type description out to a stream in
			/// SSQLS v2 form.
			void print(std::ostream& os) const;

			/// \brief Enum value accessor
			operator Value() const { return value_; }

			/// \brief Equality operator
			bool operator ==(const Type& rhs) const
					{ return value_ == rhs.value_; }

		private:
			Value value_;
		};

	private:
		std::string name_; 	///< the field's SQL name
		Type type_; 		///< the field's SQL type
		bool is_autoinc_;	///< true if DB autoincrements this column if left out of INSERT
		bool is_key_;		///< true if field is part of the primary key
		bool is_null_;		///< true if field's value is nullable
		bool is_unsigned_;	///< true if field has unsigned integer type
		std::string alias_;	///< the field's C++ name
	};

	/// \brief 'include' directive line
	class Include : public Line
	{
	public:
		/// \brief Given the name of another SSQLS v2 file, load it
		/// up and parse it.  Its contents will appear transparently
		/// as part of the overall parse result.
		///
		/// \param file_name name of other SSQLS v2 file to parse
		Include(const char* file_name) :
		pp2_(new ParseV2(file_name))
		{
		}

		/// \brief Destructor
		~Include() { pp2_->clear(); delete pp2_; }

		/// \brief Get an iterator pointing to the start of the
		// sub-parse's LineList
		LineListIt begin() const { return pp2_->begin(); }

		/// \brief Get an iterator pointing to just past the end of the
		/// sub-parse's LineList
		LineListIt end() const { return pp2_->end(); }

		/// \brief Attempt to create an Include object from information
		/// in the passed StringList
		///
		/// A kind of pre-processor for the Include ctor, creating one
		/// of those objects only if the given StringList makes sense,
		/// using the values we find in that StringList as parameters to
		/// the ctor.
		static Include* parse(const StringList& tl, bool subdirective,
				const File& file);

	private:
		// Never called.  Include directives don't appear in the parse
		// list; the included file's contents appear in its place
		// instead.  Since this method only exists to test parsing
		// behavior, we can't be called.
		void print(std::ostream&) const { assert(0); }

		/// \brief pointer to the object holding parse details for
		/// the other file we were constructed with
		ParseV2* pp2_;
	};

	/// \brief Base class for known SSQLS v2 'option' directives
	///
	/// There are subclasses for all known SSQLS v2 options, having
	/// the same name, but with different capitalization and an
	/// "Option" suffix.
	class Option : public Line
	{
	public:
		/// \brief Virtual dtor, since this is a base class
		virtual ~Option() { }

		/// \brief Attempt to create an Option object from information
		/// in the passed StringList
		///
		/// This is a kind of pre-processor for the Option subclass's
		/// ctors, creating an object of one of those subclasses only if
		/// the given StringList makes sense, passing those values as
		/// ctor parameters.
		static Option* parse(const StringList& tl, bool subdirective,
				const File& file);

	protected:
		/// \brief Protected ctor, so we cannot be directly instantiated
		///
		/// \param value the option's value
		Option(const std::string& value) :
		value_(value)
		{
		}

		/// \brief Convert a string expressing a boolean value to a \c bool
		///
		/// If we cannot recognize the value, we return true because
		/// SSQLS v2 options all default to false.  The assumption is is
		/// that if the option directive is present, the value is most
		/// likely truthy.
		///
		/// As a consequence, the only thing this function recognizes is
		/// "0", "false", "no", and "off", with any mix of upper and
		/// lower case.  All else is considered \c true.
		static bool parse_bool(const std::string& value);

		/// \brief Return the option's value in string form, unmodified
		/// from the original parse
		///
		/// Subclasses typically either expose this function in their
		/// public interface with a different, more appropriate name, or
		/// they define a wholly different method returning a reduced or
		/// more type-safe version of this value.
		const char* value() const { return value_.c_str(); }
	
	private:
		// The option's raw value string
		std::string value_;
	};

	/// \brief 'option accessor_style' directive line
	class AccessorStyleOption : public Option
	{
	public:
		/// \brief Constructor
		AccessorStyleOption(const std::string& value, const File& file) :
		Option(value),
		type_(AccessorStyleOption::parse_type(value, file))
		{
		}

		/// \brief Print the option description out to a stream in
		/// SSQLS v2 form.
		void print(std::ostream& os) const;

	private:
		/// \brief Known accessor styles
		///
		/// \internal We could implement this by deepening the Option
		/// hierarchy, making this a base class, with the known styles
		/// each implemented as a subclass, but that's seems excessively
		/// OO-dogmatic.
		enum Type {
			unknown,			///< bad style value found in .ssqls file
			camel_case_lower,	///< generate accessors like \c getX()
			camel_case_upper,	///< generate accessors like \c GetX()
			stroustrup,			///< generate accessors like \c get_x()
			overloaded			///< same method name for setter and getter
		};

		/// \brief Given a raw accessor style value straight from the
		/// parser, try to figure out which of the known styles is
		/// meant.
		static Type parse_type(const std::string& style,
				const File& file);

		/// \brief Parsed accessor style type
		Type type_;
	};

	/// \brief 'option exception_on_schema_mismatch' directive line
	class ExceptionOnSchemaMismatchOption : public Option
	{
	public:
		/// \brief Constructor
		ExceptionOnSchemaMismatchOption(const std::string& value) :
		Option(value),
		throw_(Option::parse_bool(value))
		{
		}

		/// \brief Return true if our emitted C++ code is supposed to
		/// throw an exception on schema mismatches
		operator bool() const { return throw_; }

		/// \brief Print the option description out to a stream in
		/// SSQLS v2 form.
		void print(std::ostream& os) const;

	private:
		bool throw_;	///< parsed version of parent's value
	};

	/// \brief 'option header_extension' directive line
	class HeaderExtensionOption : public Option
	{
	public:
		/// \brief Constructor
		HeaderExtensionOption(const std::string& value) :
		Option(value)
		{
		}

		/// \brief Return the extension used for C++ headers we emit
		const char* extension() const { return value(); }

		/// \brief Print the option description out to a stream in
		/// SSQLS v2 form.
		void print(std::ostream& os) const;
	};

	/// \brief 'option implementation_extension' directive line
	class ImplementationExtensionOption : public Option
	{
	public:
		/// \brief Constructor
		ImplementationExtensionOption(const std::string& value) :
		Option(value)
		{
		}

		/// \brief Return the extension used for C++ implementation
		/// files we emit
		const char* extension() const { return value(); }

		/// \brief Print the option description out to a stream in
		/// SSQLS v2 form.
		void print(std::ostream& os) const;
	};

	/// \brief 'table' directive line
	class Table : public Line
	{
	public:
		/// \brief Constructor
		///
		/// \param name the table's SQL name
		/// \param alias the table's C++ name; defaults to \c name
		/// \param filebase the base name used for generated C++ code
		///        files; defaults to \c name
		Table(const std::string& name, const std::string& alias,
				const std::string& filebase);

		/// \brief Attempt to create a Table object from information in
		/// the passed StringList
		///
		/// A kind of pre-processor for the Table ctor, creating a Table
		/// object only if the given StringList makes sense, using the
		/// values we find in that StringList as parameters to the Table
		/// ctor.
		static Table* parse(const StringList& tl, bool subdirective,
				const File& file);

		/// \brief Print the table description out to a stream in
		/// SSQLS v2 form.
		void print(std::ostream& os) const;

	private:
		std::string name_, alias_, filebase_;
	};

	//// Exception types
	/// \brief Exception object thrown to indicate a file I/O error
	class FileException : public mysqlpp::Exception
	{
	public:
		/// \brief Constructor
		///
		/// \param what description of what went wrong reading the
		/// SSQLS v2 file
		FileException(const std::string& what) : Exception(what) { }
	};

	/// \brief Exception object thrown by File::error() to report
	/// an SSQLS v2 parsing error
	class ParseException : public mysqlpp::Exception
	{
	public:
		/// \brief Constructor
		///
		/// \param what description of what went wrong parsing the line
		/// \param file_name name of source file where error occurred
		/// \param line line number in source file where error occurred
		ParseException(const std::string& what,
				const std::string& file_name, size_t line) :
		Exception(what),
		file_name_(file_name),
		line_(line)
		{
		}

		/// \brief Destructor
		~ParseException() throw() { }

		/// \brief Get name of file where error occurred
		const char* file_name() const { return file_name_.c_str(); }

		/// \brief Get line number where error occurred
		size_t line() const { return line_; }

	private:
		std::string file_name_;
		size_t line_;
	};

	/// \brief Constructor
	///
	/// Given the name of an SSQLS v2 file, load it up and try to parse
	/// it, throwing one of our inner exception types if that fails.
	///
	/// \param file_name path to an SSQLS v2 file to parse; may be in a
	///        different directory, given a relative or absolute path
	///        to the file
	///
	/// \throw FileException
	/// \throw ParseException
	ParseV2(const char* file_name);

	/// \brief Destructor
	~ParseV2()
	{
		for (LineListIt it = lines_.begin(); it != lines_.end(); ++it) {
			delete *it;
		}
	}

	/// \brief Get an iterator pointing to the start of our LineList
	LineListIt begin() const { return lines_.begin(); }

	/// \brief Dump our line list
	///
	/// Doesn't delete the line objects.  This is used by Include,
	/// because its creator will take over ownership of those objects.
	void clear() { lines_.clear(); }

	/// \brief Get an iterator pointing just past the end of our LineList
	LineListIt end() const { return lines_.end(); }

private:
	/// \brief Break line up into a series of space-separated words
	void tokenize(StringList& tokens, const std::string& line) const;

	/// \brief Information about the file we're currently parsing
	File file_;

	/// \brief List of Line object pointers representing the parse result
	///
	/// This does not contain Line sub-classes for subdirectives.  You
	/// will find a LineList inside the top-level directive class.
	///
	/// You will not find Include objects in this list corresponding to
	/// any 'include' directives in the input file.  On finishing the
	/// recursive parse for the included file, we pull its LineList
	/// contents up into this level.  This flattens the recursive parse
	/// so the code doing the traversal doesn't have to worry about it.
	LineList lines_;
};

/// \brief Write a Line out to a stream in SSQLS v2 form.
///
/// \internal This is implemented in terms of ParseV2::Line::print()
/// because operator<< needs to be a global function.  This trick lets
/// us get polymorphic behavior when writing Line objects out.
std::ostream& operator<<(std::ostream& os, const ParseV2::Line& line);

#endif // !defined(MYSQLPP_SSX_PARSEV2_H)
