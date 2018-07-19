Prerequisite: Install MySQL
~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Before you can build MySQL++, you need to install the
    libmysqlclient-devel package with Cygwin's setup.exe.

	In the past, you had to build MySQL from source, since
	there was no reliable place to get a binary version of the
	client library for Cygwin.  If you must still do this for
	some reason, here's a hint on how to build just the client
	library, since you probably will be running either a native
	Windows version of the server, or have the server installed
	on another machine entirely:

    $ ./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var \
        --infodir=/usr/share/info --mandir=/usr/share/man \
        --disable-shared --without-{debug,readline,libedit,server}


Building the Library and Example Programs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Having done that, Cygwin behaves like any other Unixy system.
	See the instructions in README-Unix.txt.
