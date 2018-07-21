/***********************************************************************
 cmdline.cpp - Command line parsing stuff used by the example and
    utility programs.  Not intended for end-user use!

 Copyright (c) 2007-2009 by Educational Technology Resources,
 Inc.  getopt() and its associated globals are the public domain
 implementation made available at the 1985 UNIFORUM conference in
 Dallas, Texas; the code is untouched except for style tweaks.
 Others may also hold copyrights on code in this file.  See the
 CREDITS.txt file in the top directory of the distribution for details.

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
#include "cmdline.h"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//// getopt ////////////////////////////////////////////////////////////
// Bring in system getopt(), or define our own.

#if defined(HAVE_POSIX_GETOPT)
#	include <unistd.h>
#elif defined(HAVE_LIBIBERTY_GETOPT)
#	include <libiberty.h>
#else
	// Need to define our own getopt() on this system
	extern "C" {
		// Using C linkage to avoid potential conflicts with system
		// getopt(), if autoconf test fails to find it.
		static const char* optarg;
		static int optind = 1;
		static int getopt(int argc, char* const argv[], const char* opts)
		{
			static int optopt;
			static int sp = 1;
			register int c;
			register const char *cp;

			if (sp == 1) {
				/* If all args are processed, finish */
				if (optind >= argc) {
					return EOF;
				}
				if (argv[optind][0] != '-' || argv[optind][1] == '\0') {
					return EOF;
				}
			}
			else if (!strcmp(argv[optind], "--")) {
				/* No more goptions to be processed after this one */
				optind++;
				return EOF;
			}

			optopt = c = argv[optind][sp];

			/* Check for invalid goption */
			if (c == ':' || (cp = strchr(opts, c)) == 0) {
				fprintf(stderr, "%s: illegal option -- %c\n", argv[0], c);
				if (argv[optind][++sp] == '\0') {
					optind++;
					sp = 1;
				}

				return '?';
			}

			/* Does this goption require an argument? */
			if (*++cp == ':') {
				/* If so, get argument; if none provided output error */
				if (argv[optind][sp + 1] != '\0') {
					optarg = &argv[optind++][sp + 1];
				}
				else if (++optind >= argc) {
					fprintf(stderr,
							"%s: option requires an argument -- %c\n", argv[0], c);
					sp = 1;
					return '?';
				}
				else {
					optarg = argv[optind++];
				}
				sp = 1;
			}
			else {
				if (argv[optind][++sp] == '\0') {
					sp = 1;
					optind++;
				}
				optarg = 0;
			}

			return c;
		}
	} // end extern "C" 
#endif // didn't find either POSIX or libiberty getopt()


////////////////////////////////////////////////////////////////////////
// Generic MySQL++-specific command line parsing mechanism

namespace mysqlpp {

//// CommandLineBase::finish_parse /////////////////////////////////////
// Called by subclass when it's finished parsing the command line.  It
// does nothing if we're not still in a "successful" state.

void
CommandLineBase::finish_parse()
{
	if (successful_) {
		const int nextras = argc_ - option_index();
		if (nextras > 0) {
			extra_args_.resize(nextras);
			for (int i = 0; i < nextras; ++i) {
				extra_args_[i] = argv_[option_index() + i];
			}
		}
	}
}


//// CommandLineBase::option_argument //////////////////////////////////
// Accessor for optarg, so caller doesn't have to know about this
// getopt() style interface.  Particualy helpful on non-POSIX systems.

const char*
CommandLineBase::option_argument() const
{
	return optarg;
}


//// CommandLineBase::option_index /////////////////////////////////////
// Accessor for optind, so caller doesn't have to know about this
// getopt() style interface.  Particuarly helpful on non-POSIX systems.

int
CommandLineBase::option_index() const
{
	return optind;
}


//// CommandLineBase::parse_next ///////////////////////////////////////
// Wrapper around getopt(), using the stuff passed to our ctor to
// construct its argument list.

int
CommandLineBase::parse_next() const
{
	return getopt(argc_, argv_, opts_);
}


//// CommandLineBase::parse_error //////////////////////////////////////
// Called by subclasses when they encounter an error in parsing.  We
// wrap up several details of handling that error: display the
// message on stderr, call the subclass's print_usage() method, and
// marks the object as no longer successful.

void
CommandLineBase::parse_error(const char* message)
{
	if (message) {
		std::cerr << message << '\n';
	}
	print_usage();
	successful_ = false;
}


////////////////////////////////////////////////////////////////////////
// Command line parser for MySQL++ examples.

namespace examples {

const char* db_name = "mysql_cpp_data";


//// examples::CommandLine ctor ////////////////////////////////////////

CommandLine::CommandLine(int argc, char* const argv[],
		const char* user, const char* pass, const char* usage_extra) :
CommandLineBase(argc, argv, "hm:p:s:u:D?"),
dtest_mode_(false),
run_mode_(0),
server_(0),
user_(user && *user ? user : 0),
pass_(pass && *pass ? pass : ""),
usage_extra_(usage_extra)
{
	int ch;
	while (successful() && ((ch = parse_next()) != EOF)) {
		switch (ch) {
			case 'm': run_mode_ = atoi(option_argument()); break;
			case 'p': pass_ = option_argument();           break;
			case 's': server_ = option_argument();         break;
			case 'u': user_ = option_argument();           break;
			case 'D': dtest_mode_ = true;                  break;
			default:
				parse_error();
				return;
		}
	}

	finish_parse();
}


//// examples::CommandLine::print_usage ////////////////////////////////
// Show a generic usage message suitable for ../examples/*.cpp  The
// parameters specialize the message to a minor degree.

void
CommandLine::print_usage(const char* extra) const
{
	std::cout << "usage: " << program_name() <<
			" [-s server_addr] [-u user] [-p password] " <<
			(extra ? extra : "") << std::endl;
	std::cout << std::endl;
	std::cout << "    If no options are given, connects to database "
			"server on localhost" << std::endl;
	std::cout << "    using your user name and no password." << std::endl;
	if (extra && (strlen(extra) > 0)) {
		std::cout << std::endl;
		std::cout << "    The extra parameter " << extra <<
				" is required, regardless of which" << std::endl;
		std::cout << "    other arguments you pass." << std::endl;
	}
	std::cout << std::endl;
}

} // end namespace mysqlpp::examples


////////////////////////////////////////////////////////////////////////
// Command line parser for MySQL++'s ssqlsxlat tool.

namespace ssqlsxlat {

//// ssqlsxlat::CommandLine ctor ////////////////////////////////////////

CommandLine::CommandLine(int argc, char* const argv[]) :
CommandLineBase(argc, argv, "hi:o:p:s:t:u:1:?"),
input_(0),
output_(0),
pass_(""),
server_(0),
user_(0),
input_source_(ss_unknown),
output_sink_(ss_unknown)
{
	// Parse the command line
	int ch;
	while (successful() && ((ch = parse_next()) != EOF)) {
		switch (ch) {
			case 'i':
			case 't':
			case '1':
				if (input_) {
					std::cerr << "Warning: overriding previous input "
							"source!  Only last -i, -t or -1 is "
							"effective.\n";
				}
				input_ = option_argument();
				input_source_ =
						(ch == '1' ? ss_ssqls1 :
						 ch == 'i' ? ss_ssqls2 :
						             ss_table);
				break;

			case 'o':
				output_ = option_argument();
				output_sink_ = ss_ssqls2;
				break;

			case 'p':
				pass_ = option_argument();
				break;

			case 's':
				server_ = option_argument();
				break;

			case 'u':
				user_ = option_argument();
				break;

			default:
				parse_error();
		}
	}
	finish_parse();

	// Figure out whether command line makes sense, and if not, tell
	// user about it.
	if (successful()) {
		if (input_source_ == ss_unknown) {
			parse_error("No input source given!  Need -i, -t or -1.");
		}
		else if ((input_source_ != ss_ssqls2) && !output_) {
			parse_error("Need -o if you give -t or -1!");
		}
	}
}


//// ssqlsxlat::CommandLine::print_usage ////////////////////////////////
// Show a generic usage message suitable for ../ssqlsxlat/*.cpp  The
// parameters specialize the message to a minor degree.

void
CommandLine::print_usage() const
{
	std::cerr << "usage: " << program_name() <<
        	" [ -i input.ssqls ] [ -1 input-ssqlsv1.cpp ]\n"
			"        [ -u user ] [ -p password ] [ -s server ] [ -t table ]\n"
			"        [ -o parsedump.ssqls ]\n";
	std::cerr << std::endl;
	std::cerr <<
			"        -i: parse SSQLSv2 DSL, generating C++ output at minimum\n"
			"        -o: write out .ssqls file containing info found by\n"
			"            processing -i, -t or -1\n"
			"  -u,p,s,t: log into server with given creds, get schema details\n"
			"            for a table, and generate output as if parsed from\n"
			"            SSQLSv2 DSL; requires -o\n"
			"        -1: find SSQLSv1 declarations in C++ code, and try to\n"
			"            interpret as equivalent SSQLSv2; requires -o\n"
			"      -?,h: write out .ssqls file containing info found by\n"
			"            processing -i, -t or -1\n";
	std::cerr << std::endl;
}

} // end namespace mysqlpp::ssqlsxlat
} // end namespace mysqlpp
