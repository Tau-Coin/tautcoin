What It Is
~~~~~~~~~~
    MySQL++ is a C++ wrapper for MySQL's C API.  It is built around STL
    principles, to make dealing with the database as easy as dealing
    with an STL container.  MySQL++ relieves the programmer of dealing
    with cumbersome C data structures, generation of repetitive SQL
    statements, and manual creation of C++ data structures to mirror
    the database schema.

    Its home page is http://tangentsoft.net/mysql++/


Prerequisites
~~~~~~~~~~~~~
    To build MySQL++, you must have the MySQL C API development
    files installed.

    On Unixy systems (Linux, Mac OS X, Cygwin, *BSD, Solaris...),
    the MySQL development files are installed if you build MySQL
    from source.  If you installed MySQL as a binary package, then
    the development files are often packaged separately from the
    MySQL server itself.  It's common for the package containing the
    development files to be called something like "MySQL-devel".

    If you're building on Windows with Visual C++ or MinGW, you
    need to install the native Win32 port of MySQL from mysql.com.
    The development files are only included with the "complete"
    version of the MySQL installer, and some versions of this
    installer won't actually install them unless you do a custom
    install.  Another pitfall is that MySQL++'s project files assume
    that you've installed the current General Availability release of
    MySQL (v5.0 right now) and it's installed in the default location.
    If you've installed a different version, or if MySQL Inc. changes
    the default location (which they seem to do regularly!) you'll have
    to adjust the link and include file paths in the project settings.


Additional Things to Read
~~~~~~~~~~~~~~~~~~~~~~~~~
    Each major platform we support has a dedicated README-*.txt
    file for it containing information specific to that platform.
    Please read it.

    For authorship information, see the CREDITS.txt file.

    For license information, see the COPYING.txt file.

    If you want to change MySQL++, see the HACKERS.txt file.

    You should have received a user manual and a reference manual
    with MySQL++.  If not, you can read a recent version online:

        http://tangentsoft.net/mysql++/doc/

    Search the MySQL++ mailing list archives if you have more
    questions:

        http://lists.mysql.com/plusplus/


Building the Library
~~~~~~~~~~~~~~~~~~~~
    MySQL++ uses Bakefile (http://bakefile.org/) to generate
    platform-specific project files and makefiles from a single set
    of input files.  We currently support these build systems:

    autoconf:
        For Unixy platforms, including Linux, Mac OS X, and Cygwin, in
        addition to the "real" Unices.  See README-Unix.txt for general
        instructions.  Supplementary platform-specific details are
        in README-Cygwin.txt, README-Linux.txt, README-Mac-OS-X.txt,
        and README-Solaris.txt.

    MinGW:
        We ship Makefile.mingw for MinGW.  It currently only builds the
        static version of the library for technical reasons.  This has
        licensing ramifications.  See README-MinGW.txt for details.

    Visual C++:
        We ship Visual Studio 2003, 2005, and 2008 project files.
        No older version of Visual C++ will build MySQL++, due to
        compiler limitations.  See README-Visual-C++.txt for more
        details.

    Xcode:
        We ship an Xcode v2 project file.  It hasn't been tested
        much yet, since the autoconf method works just fine on OS X.
        As a result, we need both success and failure reports on the
        mailing list.  See README-Mac-OS-X.txt for more information.


Example Programs
~~~~~~~~~~~~~~~~
    You may want to try out the programs in the examples subdirectory
    to ensure that the MySQL++ API and your MySQL database are both
    working properly.  Also, these examples give many examples of
    the proper use of MySQL++.  See README-examples.txt for further
    details.


Unsupported Compliers
~~~~~~~~~~~~~~~~~~~~~
    If you're on Windows but want to use some other compiler besides
    Visual C++ or GCC, you are currently on your own.  There have
    been past efforts to port MySQL++ to other Windows compilers,
    but for one reason or another, all of these ports have died.

    On Unixy systems, GCC still works best.  "Native" compilers and
    third-party compilers may work, but you're on your own to get
    it working.

    We have nothing in particular against these unsupported systems.
    We just lack the time and resources to support everything
    ourselves.  If you are sufficiently motivated to get MySQL++
    working on one of these alternate systems, see the HACKERS.txt
    file first for guidance.  If you follow the advice in that file,
    your patch will be more likely to be accepted.


If You Want to Hack on MySQL++...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    If you intend to change the library or example code, please read
    the HACKERS.txt file.

    If you want to change the user manual, read doc/userman/README.txt

    If you want to change the reference manual, see the Doxygen manual:
    http://www.stack.nl/~dimitri/doxygen/manual.html
