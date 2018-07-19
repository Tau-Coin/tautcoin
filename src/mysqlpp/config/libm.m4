dnl @synopsis LIB_MATH
dnl 
dnl This macro figures out how whether programs using C's math routines
dnl need to link to libm or not.  This is common on SysV Unices.
dnl
dnl @category C
dnl @author Warren Young <warren@etr-usa.com>
dnl @version 1.2, 2006-03-06

AC_DEFUN([LIB_MATH],
[
	AC_MSG_CHECKING([whether -lm is needed to use C math functions])
	
	MYSQLPP_EXTRA_LIBS=
	TRY_LIBM=no
	AC_TRY_LINK(
		[ #include <math.h> ],
		[ floor(0); ], AC_MSG_RESULT(no), TRY_LIBM=yes)

	if test "x$TRY_LIBM" = "xyes"
	then
		save_LIBS=$LIBS
		LIBS="$LIBS -lm"
		AC_TRY_LINK(
			[ #include <math.h> ],
			[ floor(0); ],
			[ 
				MYSQLPP_EXTRA_LIBS=-lm 
				AC_MSG_RESULT(yes)
			],
			AC_MSG_ERROR([Failed to build program containing math functions!]))
		LIBS="$save_LIBS"
	fi

	AC_SUBST(MYSQLPP_EXTRA_LIBS)
])

