For the most part, Solaris is just another Unix variant as far as
MySQL++ is concerned.  See README-Unix.txt for most of what you need
to know to build and use MySQL++.


Prerequisite: Install the MySQL Development Files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    MySQL++ is built on top of the MySQL C API library, so it needs the
    C API development files to build against.

    With the current version of Solaris, at least, a MySQL package
    is included on the operating system disk, but not installed
    by default.  To install it, oull down the GNOME System menu,
    go to Administration, and then to Package Manager.  Search for
    "mysql5" and install those packages.  While there, you may also
    need to install the gcc packages, if you haven't done that already.
    I'm not sure, but you may need to install Perl as well.

    Don't search for just "mysql" in Package Manager, as that will also
    bring up legacy MySQL 4.0 packages.  MySQL++ may build against 4.0
    still; it's been a while since we've tested it.  What is certain
    is that the examples won't run against 4.0 without modification,
    as they assume the availability of UTF-8 character set support,
    which was added in 4.1.

    It's no doubt possible to use the official binaries from mysql.com
    instead, or to build from source.  We don't do that ourselves,
    though, and don't have reports from those who have, so we can't
    advise on how to do it.


C API Development File Directories
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Sun's MySQL package installs the development files in relatively
    uncommon locations.  The libraries are in /usr/mysql/lib/mysql,
    and the headers are in /usr/mysql/include/mysql.  Way to be
    redundant, guys. :)
