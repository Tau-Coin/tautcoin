# This file is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with MySQL++; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
# USA

# Standard autotools stuff
AC_INIT(mysql++, 3.2.2, plusplus@lists.mysql.com, mysql++)
AC_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIR([config])
AC_CANONICAL_SYSTEM


# Enable libtool to decide shared library compile flags (ie -fPIC)
AC_PROG_LIBTOOL
AC_SUBST([LIBTOOL_DEPS])


# Break package version up into major, minor and bugfix components.
MYSQLPP_VERSION_MAJOR=`echo $PACKAGE_VERSION | cut -f1 -d.`
AC_SUBST(MYSQLPP_VERSION_MAJOR)
MYSQLPP_VERSION_MINOR=`echo $PACKAGE_VERSION | cut -f2 -d.`
AC_SUBST(MYSQLPP_VERSION_MINOR)
MYSQLPP_VERSION_BUGFIX=`echo $PACKAGE_VERSION | cut -f3 -d.`
AC_SUBST(MYSQLPP_VERSION_BUGFIX)


# Check for Standard C support
AC_PROG_CC
AC_HEADER_STDC


# Figure out whether/how to handle threading support, if available.
AC_ARG_ENABLE(thread-check,
		[  --enable-thread-check  Check for threads, and use if available. ],
		[ thread_check=yes ])
if test "x$thread_check" = "xyes"
then
	ACX_PTHREAD
	LIBS="$PTHREAD_LIBS $LIBS"
	CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
	CC="$PTHREAD_CC"
	AC_CHECK_HEADERS(synch.h)
	AC_CHECK_HEADERS(unistd.h)
fi


# Let caller provide -f to lib/*.pl scripts in a uniform way
AC_ARG_WITH([field-limit],
		AS_HELP_STRING([--with-field-limit=<n>],
			[set max template query and SSQLS field count]),
		[], [])
if test -n "$with_field_limit"
then
	 ( cd lib ;
	   ./querydef.pl -f $with_field_limit ;
	   ./ssqls.pl -f $with_field_limit 
	 )
fi


# Try to find local getopt(); if we fail, we'll use the one in lib/cmdline.*
AC_CHECK_FUNC(getopt, 
		[AC_DEFINE(HAVE_POSIX_GETOPT, [], Define if getopt() is available in unistd.h)],
		[AC_CHECK_LIB(iberty, getopt,
			[AC_DEFINE(HAVE_LIBIBERTY_GETOPT, [], Define if getopt() is available in libiberty.h)],
			[])])


# Checks for libraries and local system features
LIB_MATH
LIB_SOCKET_NSL
MYSQL_C_API_LOCATION
MYSQL_WITH_SSL
AX_C_LOCALTIME_R
AC_CHECK_LIB(intl, main)


# If this is Cygwin, add a linker flag to suppress a silly link message.
case "${host}" in
	*cygwin*)
		LDFLAGS="$LDFLAGS -Wl,--enable-auto-import"
		;;
esac


# Check for Standard C++ support, and extensions.  This must be near
# the end, because the CPLUSPLUS directive makes autoconf use C++
# compiler for all subsequent tests!
AC_PROG_CXX
AC_LANG_CPLUSPLUS
STL_SLIST_EXTENSION


# Include Bakefile macros
AC_BAKEFILE([m4_include(config/autoconf_inc.m4)])


#
# Configure process complete; write out files generated from *.in.
#
AC_OUTPUT([\
	doc/userman/userman.dbx \
	install.hta \
	lib/Doxyfile \
	lib/mysql++.h \
	ssx/Doxyfile \
	abi.xml \
	mysql++.spec \
	Makefile \
	version
])
chmod +x version

