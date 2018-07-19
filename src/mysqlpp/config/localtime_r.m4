dnl @synopsis AX_C_LOCALTIME_R
dnl 
dnl This macro determines whether the C runtime library contains
dnl localtime_r(), a thread-safe replacement for localtime().
dnl
dnl @version 1.0, 2007/02/20
dnl @author Warren Young <mysqlpp@etr-usa.com>
AC_DEFUN([AX_C_LOCALTIME_R],
[
	AC_MSG_CHECKING([for localtime_r()])

	AC_TRY_RUN([
		#include <time.h>
		int main(void)
		{
			time_t tt;
			struct tm stm;
			localtime_r(&tt, &stm);
			return 0;
		}
	], [localtime_r_found=yes], [localtime_r_found=no], [localtime_r_found=no])

	AC_MSG_RESULT([$localtime_r_found])
	if test x"$localtime_r_found" = xyes
	then
		AC_DEFINE(HAVE_LOCALTIME_R, 1,
			[Define if you have the localtime_r() facility])
	fi
]) dnl AX_C_LOCALTIME_R

