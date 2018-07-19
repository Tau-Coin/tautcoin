Linux is basically Unix, so README-Unix.txt covers the generic bits.
I'll just cover a few of the issues specific to Linux here.


Prerequisite: Install the MySQL Development Files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    MySQL++ is built on top of the MySQL C API library, so it needs the
    C API development files to build against.  Most distributions of
    the MySQL server for Linux don't come with these development files;
    they're packaged separately.  This is because you don't actually
    need the server on your development machine, though it's often more
    convenient to use a local server than a remote one, for testing.

    There are about as many different ways to get the C API development
    files on your system as there are major Linux distributions.
    More actually, because you also have the option of the official
    MySQL binaries from mysql.com:

        http://dev.mysql.com/downloads/mysql/5.0.html#linux

    For RPM-based distributions, MySQL comes in several different
    packages.  You need at least the -devel and the -shared packages
    to build MySQL++.

    The other binary distributions seem to come in just a single file,
    presumably with everything included.

    You can also build from source, in which case you will also get
    the entire kit and kaboodle.

    MySQL frequently comes with Linux distributions as well.  If your
    distribution doesn't come with at least MySQL v4.1, I recommend
    using the official MySQL.com packages instead.  MySQL++ can be
    made to run with 4.0 and older, but it takes some work.

    On Red Hat type systems with yum, say:

        # yum install mysql-devel

    If you want to use rpm directly, you need that package and probably
    the base mysql package as well.

    On Debian/Ubuntu type systems, say:

        $ sudo apt-get install libmysqlclient15-dev

    The version number is the ABI version of the MySQL C API library.
    ABI version 15 corresponds to MySQL version 5.0, the recommended
    stable version as of this writing.

        
Dealing with the Dynamic Linker, ld.so
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ld.so is a system-level program on Linux which is used to run any
    program that uses shared libraries (lib*.so).  Its job is to find
    the libraries and link them to the base executable so it will run.

    Because ld.so only looks in a few places for libraries on most
    systems, a common problem is a program that builds without error
    but won't run, complaining about libmysqlpp.SOMETHING.

    There are a number of ways to deal with this.

    First, you could just configure MySQL++ to install under /usr
    instead of /usr/local, like system-provided packages:

        $ ./configure --prefix=/usr

    This isn't recommended practice when building packages from source,
    but it does work.

    Second, you can add the MySQL++ library directory to the
    LD_LIBRARY_PATH environment variable.  This works like the shell's
    PATH variable: a colon-separated list of directories to search.
    This is best when the installation directory is something totally
    uncommon, or you don't have root permissions on the box so you
    can't do the next option.

    Finally, the most robust way to tell ld.so about a nonstandard
    library directory is to put it in /etc/ld.so.conf or in one of
    the files included from there.  Then, run ldconfig as root to
    rebuild the cache file ld.so uses to find libraries in these
    nonstandard directories.  Running ldconfig isn't necessary for
    the previous two methods.
