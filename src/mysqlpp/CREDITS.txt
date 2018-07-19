MySQL++ was created by Kevin Atkinson during 1998.  From version
1.0 (released in June 1999) through 1.7.9 (May 2001), the primary
maintainer was Sinisa Milivojevic <sinisa@mysql.com>.  Neither Kevin
nor Sinisa are currently involved in MySQL++ development.  The current
maintainer is Warren Young <mysqlpp@etr-usa.com>, starting with
version 1.7.10 in August of 2004.

For a fuller account of the library's history, see the first chapter
of the user manual.  For the nitty-gritty details, see the ChangeLog
in the root package directory.  ChangeLog items since 1.7.9 that
aren't attributed to anyone else were done by Warren Young.


Other contributors of note since 1.7.10:

    Chris Frey <cdfrey@foursquare.net>: Lots of GCC warning fixes
    for the bleeding-edge compiler versions, Gentoo ebuild support,
    and misc other fixes.

    Mark Meredino <Mark_Merendino@cnt.com>: Several fixes and
    additions, including a lot of work on Microsoft Visual C++
    compatibility, and discoveries made while spelunking in the
    library.

    Evan Wies <evan@athenacr.com>: Contributed several C++ code
    style cleanups.

    Arnon Jalon <Arnon.Jalon@247RealMedia.com>: Added the multi-query
    result set handling features, and examples/multiquery.cpp to
    demonstrate it.

    Korolyov Ilya has submitted several patches in many different
    areas of the library.

    Remi Collet <Liste@FamilleCollet.com> is maintaining offical RPMs
    for Fedora, with other systems on the way.  His work has improved
    the RPM spec file we distribute greatly.

    Joel Fielder <joel.fielder@switchplane.com> of Switchplane,
    Ltd. created the ScopedConnection class, came up with the original
    idea for Query's for_each() and store_in() methods, provided the
    basis for examples/for_each.cpp, and provided a fix for exception
    flag propagation in Query.

    Jim Wallace <jwallace@kaneva.com> demonstrated the need
    for BadQuery::errnum(), and contributed the patches and also
    examples/deadlock.cpp to test that this feature does what it is
    supposed to.

    Jonathan Wakely <mysql@kayari.org> rebuilt my original versions
    of ConnectionPool, RefCountedPointer, and RefCountedBuffer.
    They're now simpler and safer.  He also created the numeric
    conversion logic in lib/mystring.h introduced in v3.0.

    Adrian Cornish <mysql@bluedreamer.com>  Several fixes and
    additions.

    Rick Gutleber <rgutleber@above.net> contributed the 
    Query::insertfrom() method and associated InsertPolicy object,
    as well as the SQLStream class.

Here are the personal credits from the old 1.7.9 documentation,
apparently written by Kevin Atkinson:

    Chris Halverson - For helping me get it to compile under Solaris.

    Fredric Fredricson - For a long talk about automatic conversions.

    Michael Widenius - MySQL developer who has been very supportive of
        my efforts.

    Paul J. Lucas - For the original idea of treating the query object
        like a stream.

    Scott Barron - For helping me with the shared libraries.

    Jools Enticknap - For giving me the Template Queries idea.

    M. S. Sriram - For a detailed dission of how the Template Queries
        should be implemented, the suggestion to throw exceptions on bad
        queries, and the idea of having a back-end independent query
        object (ie SQLQuery).

    Sinisa Milivojevic - For becoming the new offical maintainer.

    D. Hawkins and E. Loic for their autoconf + automake contribution.


See the ChangeLog for further credits, and details about the differences
between the many versions of this library.


Please do not email any of these people with general questions about
MySQL++. All of us who are still active in MySQL++ development read the
mailing list, so questions sent there do get to us:

    http://lists.mysql.com/plusplus

The mailing list is superior to private email because the answers are
archived for future questioners to find, and because you are likely to
get answers from more people.
