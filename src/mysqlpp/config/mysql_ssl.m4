dnl @synopsis MYSQL_WITH_SSL
dnl
dnl This macro determines whether mysql_ssl_set() API call exists.
dnl Requires at least MySQL 4.0.1. 
dnl 
dnl @version $Id$, $Date$
dnl @author Ovidiu Bivolaru <ovidiu@targujiu.rdsnet.ro>
AC_DEFUN([MYSQL_WITH_SSL],
[
    #
    # Check for mysql_ssl_set() in libmysqlclient(_r)
    #
    AC_CHECK_LIB($MYSQL_C_LIB_NAME, mysql_ssl_set, [
	AC_DEFINE(HAVE_MYSQL_SSL_SET,, Define if your MySQL library has SSL functions)
    ]) dnl AC_CHECK_LIB(mysqlclient, mysql_ssl_set)
]) dnl  MYSQL_WITH_SSL

