dnl @synopsis LIB_SOCKET_NSL
dnl 
dnl This macro figures out what libraries are required on this platform
dnl to link sockets programs.
dnl
dnl The common cases are not to need any extra libraries, or to need
dnl -lsocket and -lnsl.  We need to avoid linking with libnsl unless
dnl we need it, though, since on some OSes where it isn't necessary it
dnl will totally break networking.  Unisys also includes gethostbyname()
dnl in libsocket but needs libnsl for socket().
dnl
dnl @category Misc
dnl @author Warren Young <mysqlpp@etr-usa.com>
dnl @version 1.5, 2006-03-06

AC_DEFUN([LIB_SOCKET_NSL],
[
	save_LIBS="$LIBS"

	AC_MSG_CHECKING([whether -lsocket is needed])
	TRY_LSOCKET=no
	AC_TRY_LINK(
		[ 
			#include <sys/types.h>
			#include <sys/socket.h> 
			#include <netinet/in.h>
			#include <arpa/inet.h>
		],
		[ socket(AF_INET, SOCK_STREAM, 0); ],
		AC_MSG_RESULT(no), TRY_LSOCKET=yes)

	if test "x$TRY_LSOCKET" = "xyes"
	then
		LIBS="-lsocket $LIBS"
		AC_TRY_LINK(
			[ 
				#include <sys/types.h>
				#include <sys/socket.h> 
				#include <netinet/in.h>
				#include <arpa/inet.h>
			],
			[ socket(AF_INET, SOCK_STREAM, 0); ],
			[ 
				MYSQLPP_EXTRA_LIBS="-lsocket $MYSQLPP_EXTRA_LIBS"
				AC_MSG_RESULT(yes)
			],
			AC_MSG_ERROR([failed to link using -lsocket!]))
	fi

	AC_MSG_CHECKING([whether -lnsl is needed])
	TRY_LNSL=no
	AC_TRY_LINK(
		[ #include <netdb.h> ],
		[ gethostbyname("gna.org"); ],
		AC_MSG_RESULT(no), TRY_LNSL=yes)

	if test "x$TRY_LNSL" = "xyes"
	then
		LIBS="-lnsl $LIBS"
		AC_TRY_LINK(
			[ #include <netdb.h> ],
			[ gethostbyname("gna.org"); ],
			[ 
				MYSQLPP_EXTRA_LIBS="-lnsl $MYSQLPP_EXTRA_LIBS"
				AC_MSG_RESULT(yes)
			],
			AC_MSG_ERROR([failed to link using -lnsl!]))
	fi

	AC_SUBST(MYSQLPP_EXTRA_LIBS)
])

