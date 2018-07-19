dnl @synopsis STL_SLIST_EXTENSION
dnl 
dnl This macro determines whether the local STL implementation includes
dnl a singly-linked list template, slist, and if so, where it is.
dnl
dnl @version 1.2, 2005/07/22
dnl @author Warren Young <mysqlpp@etr-usa.com>
AC_DEFUN([STL_SLIST_EXTENSION],
[
	AC_MSG_CHECKING([for STL slist extension])

	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
			[#include <slist>],
			[slist<int> l])],
		AC_DEFINE(HAVE_GLOBAL_SLIST, 1,
			[ Define if you have ::slist container in <slist> ]),
		TRY_NEXT=yes)

	if test -z "$TRY_NEXT"
	then
		SLIST_LOC="<slist>, global scope"
	else
		TRY_NEXT=""
		AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM(
				[#include <slist>],
				[std::slist<int> l])],
			AC_DEFINE(HAVE_STD_SLIST, 1,
				[ Define if you have std::slist container in <slist> ]),
			TRY_NEXT=yes)

		if test -z "$TRY_NEXT"
		then
			SLIST_LOC="<slist>, namespace std"
		else
			TRY_NEXT=""
			AC_COMPILE_IFELSE(
				[AC_LANG_PROGRAM(
					[#include <ext/slist>],
					[__gnu_cxx::slist<int> l])],
				AC_DEFINE(HAVE_EXT_SLIST, 1,
					[ Define if you have __gnu_cxx:slist container in <ext/slist> ]),
				SLIST_LOC="not found")

			if test -z "$SLIST_LOC"
			then
				SLIST_LOC="<ext/slist>, namespace __gnu_cxx"
			fi
		fi
	fi

	AC_MSG_RESULT([$SLIST_LOC])
]) dnl STL_SLIST_EXTENSION

