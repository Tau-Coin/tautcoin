If you are going to make any changes to MySQL++, this file has some
hints and commentary you may find helpful.


Subversion Access
~~~~~~~~~~~~~~~~~
    To check out the current development version from the Gna!
    Subversion repository, say:

        $ svn co svn://svn.gna.org/svn/mysqlpp/trunk mysqlpp

    If you're a MySQL++ committer, use svn over ssh instead:
    
        $ svn co svn+ssh://LOGIN@svn.gna.org/svn/mysqlpp/trunk mysqlpp

    where LOGIN is your Gna! login name.  You will have to have your
    ssh public key(s) registered with Gna! for this to work.


Bootstrapping the Library
~~~~~~~~~~~~~~~~~~~~~~~~~
    When you check out MySQL++ from svn, there are a lot of things
    "missing" as compared to a distributed tarball, because the
    svn repository contains only source files, no generated files.
    The process that turns a fresh MySQL++ repository checkout into
    something you can build and hack on is called bootstrapping.

    Boostrapping is best done on a modern Unix type platform: Linux, OS
    X, BSD, Solaris...any version released in the last 4 years or so.
    It's possible to do it on Windows, but harder; enough so that we
    cover the options below in a separate section.

    Two of the tools you need to do this are commonly available on
    Unixy systems, at least as an option: Perl 5, and autoconf 1.59
    or higher.  If they're not installed, you can probably run your
    system's package manager to install suitable versions.

    There's a third tool you'll need to bootstrap MySQL++, called
    Bakefile, which you can get from http://bakefile.org/  You will
    need Bakefile 0.2.5 or higher, which in turn requires Python 2.3
    or higher to run.  To build Bakefile from source, you will also
    need SWIG, so if you don't have that, you'll want to use one of
    the binary builds of Bakefile.

    Once you have all the tools in place, you can bootstrap MySQL++
    with a Bourne shell script called bootstrap, which you get as part
    of the svn checkout.  It's fairly powerful, with many options.
    For most cases, it suffices to just run it without any arguments:

        $ ./bootstrap

    For more unusual situations, here's the complete usage:

        $ ./bootstrap [no{doc,ex,lib,opt}] [pedantic] [bat] \
                      [configure flags]

    Arguments:

    nodoc   The documentation won't be considered a prerequisite for
            building the distribution tarball.  This is useful on systems
            where the documentation doesn't build correctly, and you only
            need to make a binary RPM.  That process requires a tarball,
            but doesn't need the documentation.  Don't distribute the
            tarball or SRPM that results, as they are no good for any
            other purpose.

    noex    The generated Makefiles and project files won't try to build
            any of the examples.

    nolib   The generated Makefiles and project files won't try to build
            the MySQL++ library.

    nomaint Turn off "maintainer mode" stuff in the build.  These are
            features used only by those building MySQL++ from svn.  The
            'dist' build target uses this when creating the tarball.

    noopt   Compiler optimization will be turned off.  (This currently
            has no effect on MinGW or Visual C++.)

    pedantic
            Turns on all of GCC's warnings and portability checks.
            Good for checking changes before making a public release.

    bat     Asks cmd.exe to run bootstrap.bat for you.  This is useful
            when using Cygwin just as a command shell in preference
            to cmd.exe, as opposed to using Cygwin to build MySQL++
            using its native tools.  Passing 'bat' stops all command
            line processing in the bootstrap script, so if you
            also pass some of the other options, make 'bat' last.
            The only options that affect the built project files and
            Makefiles work are the no* ones.

    configure options
            As soon as the bootstrap script sees an option that it
            doesn't understand, it stops processing the command line.
            Any subsequent options are passed to the configure script.
            See README-Unix.txt for more on configure script options.


Bootstrapping the Library Using only Windows
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    The thing that makes bootstrapping on Windows difficult is that
    one of the required steps uses a Unix-centric tool, autoconf.
    This section is about working out a way to get that working on
    Windows, or avoiding the need for it, so you can get on with
    hacking on MySQL++ on Windows.

    The thing autoconf does that's relevant to Windows builds
    of MySQL++ is that it substitutes the current MySQL++ version
    number into several source files.  This allows us to change the
    version number in just one place -- configure.ac -- and have
    it applied to all these other places.  Until you do this step,
    an svn checkout of MySQL++ won't build, because these files with
    the version numbers in them won't be generated.

    Option 1: Copy the generated files over from a released version
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Only one of these generated files is absolutely critical to
        allowing MySQL++ to build: lib/mysql++.h.  So, the simplest
        option you have to bootstrap MySQL++ entirely on Windows is to
        copy lib/mysql++.h over from a released version of MySQL++.
        While you're doing that, you might copy over the other such
        generated files:

            install.hta
            mysql++.spec
            doc/userman/userman.dbx
            lib/Doxyfile

        Having done that, you can complete the bootstrapping process by
        running bootstrap.bat.  It has the same purpose as the Bourne
        shell script described above, but much simpler.  It has none
        of the command line options described above, for one thing.

        The main downside of doing it this way is that your changed
        version will have the same version number as the release of
        MySQL++ you copied the files from, unless you go into each
        file and change the version numbers.

    Option 2: Cygwin
    ~~~~~~~~~~~~~~~~
        If you'd like to hack on MySQL++ entirely on Windows and
        have all the build freedoms enjoyed by those working on Unixy
        platforms, the simplest solution is probably to install Cygwin.

        Get the Cygwin installer from http://cygwin.com/setup.exe

        When you run it, it will walk you through the steps to
        install Cygwin.  Autoconf and Perl 5 aren't installed in
        Cygwin by default, so when you get to the packages list,
        be sure to select them.  Autoconf is in the Devel category,
        and Perl 5 in the Interpreters category.

        You will also need to install the native Windows binary
        version of Bakefile, from http://bakefile.org/  Don't get
        the source version and try to build Bakefile under Cygwin; it
        won't work.  The Windows binary version of Bakefile includes
        an embedded version of Python, so you won't need to install
        Cygwin's Python.
        
        Having done all this, you can follow the Unix bootstrapping
        instructions in the previous section.

    Option 3: "Here's a nickel, kid, get yourself a better computer."
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        http://tomayko.com/writings/that-dilbert-cartoon

        Finally, you might have access to a Unixy system, or the
        ability to set one up.  You don't even need a separate physical
        computer, now that virtual machine techology is free.

        For example, you could download a Linux "appliance" from
        http://www.vmware.com/appliances/ and a copy of the free
        VMware Player to run it on your Windows machine.  You'd do the
        svn checkout of MySQL++ on that machine, bootstrap it there
        using the instructions in the previous section.
        
        That done, just copy the result over to the Windows machine
        to continue hacking on it.


On Manipulating the Build System Source Files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    One of the things the bootstrapping system described above
    does is produces various types of project and make files from a
    small number of source files.  This system lets us support many
    platforms without having to maintain separate build system files
    for each platform.

    Bakefile (http://bakefile.org/) produces most of these project
    and make files from a single source file called mysql++.bkl.

    Except for small local changes, it's best to change mysql++.bkl
    and "re-bake" the project and make files rather than change
    those files directly.  You can do this with the bootstrap scripts
    covered above.  On Windows, if all you've changed is mysql++.bkl,
    you can use rebake.bat instead, which doesn't try to do as much
    as bootstrap.bat.

    Bakefile produces finished project files for Visual C++ and Xcode
    and finished Makefiles for MinGW.  It also produces Makefile.in,
    which is input to GNU Autoconf along with configure.ac
    and config/*.  You may need to change these latter files in
    addition to or instead of mysql++.bkl to get the effect you want.
    Running bootstrap incorporates changes to all of these files in
    the GNU autoconf output.

    While Bakefile's documentation isn't as comprehensive as it
    ought to be, you can at least count on it to list all of the
    available features.  So, if you can't see a way to make Bakefile
    do something, it's likely it just can't do it.  Bakefile is a
    high-level abstraction of build systems in general, so it'll never
    support all the particulars of every odd build system out there.


Submitting Patches
~~~~~~~~~~~~~~~~~~
    If you wish to submit a patch to the library, please send it to
    the MySQL++ mailing list, or attach it to an entry in our bug
    tracker on Gna!  We want patches in unified diff format.

    The easiest way to get a unified diff is to check out a copy of the
    current MySQL++ tree as described above.  Then make your change,
    cd to the MySQL++ root directory, and ask Subversion to generate
    the diff for you:

        $ svn diff > mychange.patch

    If your patch adds new files to the distribution, you can say
    "svn add newfile" before you do the diff, which will include
    the contents of that file in the patch.  (You can do this even
    when you've checked out the tree anonymously.)  Then say "svn
    revert newfile" to make Subversion forget about the new file.

    If you're making a patch against a MySQL++ distribution tarball,
    then you can generate the diff this way:

        $ diff -ruN mysql++-olddir mysql++-newdir > mychange.patch

    The diff command is part of every Unix and Linux system, and
    should be installed by default.  If you're on a Windows machine,
    GNU diff is part of Cygwin (http://cygwin.com/).  Subversion is
    also available for all of these systems.  There are no excuses
    for not being able to make unified diffs.  :)


The MySQL++ Code Style
~~~~~~~~~~~~~~~~~~~~~~
    Every code base should have a common code style.  Love it or
    hate it, here are MySQL++'s current code style rules:

    Source Code
    ~~~~~~~~~~~
        File types: ac, cpp, h, in, m4, pl

        - Tabs for indents, size 4

        - Unix line endings.  Any decent programmer's editor can
          cope with this, even on Windows.

        - C/C++ rules:

            - Base whitespace style is AT&Tish: K&R/Stroustrup,
              plus a little local spice.  If you have the indent(1)
              program, the command is:

                indent -kr -nce -cli4 -ss -di1 -psl -ts4 FILES...

              That is, don't cuddle else, indent case statement labels,
              space before semicolon with empty loop body, no extra
              space between a variable type and name, return value
              of function on separate line from rest of definition.

            - Class names are in CamelCase, uppercased first letter

            - Method names are in all_lower_case_with_underscores();
              ditto most other global symbols.

            - Macro names are in ALL_UPPERCASE_WITH_UNDERSCORES

            - Doxygen comment for all public declarations, unless
              there is a very good reason to keep the thing
              undocumented.

        - Perl and shell script rules are more or less the same
          as for C/C++, to the extent this makes sense.


    XML/HTML Dialects
    ~~~~~~~~~~~~~~~~~
        File types: bkl, dbx, hta

        - Spaces for indents, size 2.  Shallow indents due to the
          high level of nesting occurring in such files, and spaces
          because they're not as annoying at shallow indent levels
          in editors that don't treat space indents like tabs.

        - Unix line endings.  Again, these are intended to be viewed
          in a programmer's text editor, which should work with Unix
          line endings no matter the platform.


    Plain Text Files
    ~~~~~~~~~~~~~~~~
        File types: txt

        - Spaces for indents, size 4.  Spaces because such files
          are often viewed in Notepad and similarly crippled text
          editors which use a default indent level of 8.

        - DOS line endings, again for the Notepad reason.  And on
          modern Unixy platforms, the tools cope with DOS line endings
          reasonably well.  Better than the converse, anyway.


    When in doubt, mimic what you see in the current code.  When still
    in doubt, ask on the mailing list.


Testing Your Proposed Change
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    MySQL++ includes a self-test mechanism called dtest.  It's a
    Bourne shell script, run much like exrun:

        $ ./dtest [-s server_addr] [-u user] [-p password]

    This automatically runs most of the examples, captures the outputs
    to a file, and then compares that to a known-good run's outputs,
    stored in bmark.txt.  So, before you submit a patch, run dtest
    to see if anything has changed.  If something has and you can't
    account for it, it represents a problem that you'll have to fix
    before submitting the patch.  If it gives an expected change,
    remove bmark.txt, re-run dtest, and include the bmark.txt diffs in
    your patch.  This communicates to us the fact that you know there
    are differences and want the patch evaluated anyway.  Otherwise,
    we are likely to view the change as a bug.

    dtest also runs all of the unit tests in test/*.  The purpose of
    test/* is different from that of examples/*:

        - test/* are unit tests: each tests only one MySQL++ class,
          independent of everything else.  Because DB access requires
          several MySQL++ classes to cooperate, a unit test never
          accesses a database; hence, no unit test needs DB connection
          parameters.  We will never get 100% code coverage from
          test/* alone.

        - examples/* can be thought of as integration tests: they
          test many pieces of MySQL++ working together, accessing
          a real database server.  In addition to ensuring that all
          the pieces work together and give consistent results from
          platform to platform and run to run, it also fills in gaps
          in the code coverage where no suitable test/* module could
          be created.

        - test/* programs always run silently on success, writing
          output only to indicate test failures.  This is because
          they're usually only run via dtest.

        - examples/* are always "noisy," regardless of whether they
          succeed or fail, because they're also run interactively by
          people learning to use MySQL++.

    Patches should include tests if they introduce new functionality
    or fix a bug that the existing test coverage failed to catch.
    If the test is noisy, needs DB access, or tests multiple parts
    of the library at once, it goes in examples/*.  If your change
    affects only one class in MySQL++ and testing it can be done
    without instantiating other MySQL++ classes -- other than by
    composition, of course -- it should go in test/*.

    In general, prefer modifying an existing examples/* or test/*
    program.  Add a new one only if you're introducing brand new
    functionality or when a given feature currently has no test at all.

    Beware that the primary role the examples is to illustrate points
    in the user manual.  If an existing example does something similar
    to what a proper test would need to do and the test doesn't change
    the nature of the example, don't worry about changing the example
    code.  If your test would change the nature of the example, you
    either need to do the test another way, or also submit a change
    to doc/userman/*.dbx that incorporates the difference.


Adding Support for a Different Compiler
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    As described above, MySQL++ uses the Bakefile system for creating
    project files and makefiles.  This allows us to make changes
    to a single set of files, and have the proper changes be made
    to all generated project files and makefiles.  In the past, we
    used more ad-hoc systems, and we'd frequently forget to update
    individual project files and makefiles, so at any given time,
    at least one target was likely to be broken.

    If MySQL++ doesn't currently ship with project files or makefiles
    tuned for your compiler of choice, you need to work through the
    Bakefile mechanism to add support.  We're not willing to do ad-hoc
    platform support any more, so please don't ask if you can send
    us project files instead; we don't want them.

    If you want to port MySQL++ to another platform, we need to be
    confident that the entire library works on your platform before
    we'll accept patches.  In the past, we've had broken ports that
    were missing important library features, or that crashed when built
    in certain ways.  Few people will knowingly use a crippled version
    of MySQL++, since there are usually acceptable alternatives.
    Therefore, such ports become maintenance baggage with little
    compensating value.


Maintaining a Private CVS Repository
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    You may find it helpful to maintain your own CVS repository.
    Whenever there is a new MySQL++ release, import it on the vendor
    branch like this:

        $ cvs import -m "Version 1.7.35" software/mysql++ mysql++ mysql++-1_7_35

    (This assumes that you have your CVSROOT environment variable
    set properly.)

    Update the HEAD branch like this:

        $ cd mysql++
        $ cvs update -PdA
        $ cvs update -j HEAD -j mysql++-1_7_35 -Pd
        $ cvs ci -m "merged 1.7.35 into HEAD"
        $ cvs tag mysql++-1_7_35-merged

    Then any changes you make can easily be tracked, and diffs can
    be produced with rdiff:

        $ cvs rdiff -ru mysql++-1_7_35 -r mysql++-1_7_35_equal_list \
            $(cat CVS/Repository) > equal_list.patch

