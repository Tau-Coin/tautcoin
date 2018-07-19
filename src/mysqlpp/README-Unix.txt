Platform Variations
~~~~~~~~~~~~~~~~~~~
    This file only covers details common to all Unix variants
    supported by MySQL++.  For platform-specific details, see the
    file appropriate to your OS:

        README-Cygwin.txt
        README-Linux.txt
        README-Mac-OS-X.txt
        README-Solaris.txt

    There are no special instructions for any other Unix flavors.


Building the Library and Example Programs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    MySQL++ uses GNU autoconf, so you can build it with the standard
    commands:

    $ ./configure
    $ make
    $ su
    # make install


Configure Options
~~~~~~~~~~~~~~~~~
    The configure script takes several interesting options. Say:

        $ ./configure --help

    to get a list.  Some of the more interesting flags are:

    --prefix:

        If you wish to install mysql++ in a root directory other than
        /usr/local, run configure with --prefix=/some/dir/name

    --with-mysql*:

        If you installed MySQL in an atypical location, the configure
        script will not be able to find the library and header
        files without help.  The simplest way to clue configure into
        where MySQL is installed is with the --with-mysql option.
        Try something like "--with-mysql=/usr/local/mysql", for
        instance.  The configure script will then try to guess which
        subdirectories under the given directory contain the library
        and include files.

        If that doesn't work, it's because the library and header
        files aren't in typical locations under the directory you gave
        for configure to find them.  So, you need to specify them
        separately with --with-mysql-include and --with-mysql-lib
        instead.  As with --with-mysql, configure can often guess
        which subdirectory under the given directory contains the
        needed files, so you don't necessarily have to give the full
        path to these files.
    
    --with-field-limit:

        This lets you increase the maximum field limit for template
        queries and SSQLSes.  By default, both are limited to 25
        fields.  See chapter 8.2 in the user manual for details:

        http://tangentsoft.net/mysql++/doc/html/userman/configuration.html

    --enable-thread-check:

        Builds MySQL++ with threading support, if possible.
        
        This option simply turns on two tests: first, that your
        system uses a compatible threading library; and second,
        that the thread-safe version of the MySQL C API library
        (libmysqlclient_r) is installed and working.  If both of
        these are true, you get a thread-aware version of MySQL++.
        "Thread-aware" means that the library does make an effort to
        prevent problems, but we don't guarantee that all possible
        uses of MySQL++ are thread-safe.

        Note that this is a suggestion, not a command.  If we can't
        figure out the system's threading model or can't find the
        thread-aware build of the C API library, configure won't fail.
        It just reverts to the standard single-thread build.

        See the chapter on threading in the user manual for more
        details and advice on creating thread-safe programs with
        MySQL++.


Building a Static Library
~~~~~~~~~~~~~~~~~~~~~~~~~
    As shipped, MySQL++ only builds a shared library.  It's possible to
    change things so you get a static library instead.

    Before we get to "how," beware that liking statically to MySQL++ has
    legal consequences that may matter to you, due to the library's
    license, the GNU LGPL.  Familiarize yourself with the license, and
    consider getting legal counsel before proceeding.  Also, see the
    MySQL++ FAQ: http://tangentsoft.net/mysql++/#faq  There is more on
    this topic there.

    The necessary changes are all in mysql++.bkl:

        - Change the <dll> tag to <lib>.  (Remember the closing tag!)

        - Remove the <dllname> tag

        - Remove the <so_version> tag

    Then, re-bootstrap the library.  See HACKERS.txt if you need further
    instruction on doing that.
