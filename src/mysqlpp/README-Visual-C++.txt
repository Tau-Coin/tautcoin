Prerequisites
~~~~~~~~~~~~~
    You need to have the MySQL C API development files on your system,
    since MySQL++ is built on top of it.

    The easiest way to get it is to download Connector/C from
    mysql.com.

    If you need the MySQL server on your development system anyway,
    you you can choose to install the development files along with
    the server.  Some versions of the MySQL Server installer for
    Windows have installed the development files by default, while
    others have made it an optional install.


Project Files
~~~~~~~~~~~~~
    The distribution comes with three sets of .sln and .vcproj files
    in the vc2003, vc2005 and vc2008 subdirectories.

    We do this for several reasons:

      1. It lets you build MySQL++ with multiple versions of Visual
         C++ without the build products conflicting.

      2. For Visual C++ 2003, we had to disable the SSQLS feature
         because changes made in MySQL++ 3.0 now cause the compiler
         to crash while building.  See the Breakages chapter in the
         user manual for workarounds if you must still use VC++ 2003.

      3. The VC++ 2008 project files get built for 64-bit output, while
         the other two build 32-bit executables.

         With VC++ 2003, we have no choice about this, since it only
         supports 32-bit targets.

         VC++ 2005 did have experimental 64-bit compilers available,
         but their beta nature was only one reason we chose not to
         use them.  The real reason is that the current MySQL++ build
         system isn't currently set up to make it easy to build both
         32- and 64-bit libraries and executables at the same time
         within the same solution.  Bakefile allows it, but it would
         require forking many of the build rules in mysql++.bkl so
         we can do things like have separate MYSQL_WIN_DIR values
         for each bitness.  (See below for more on this variable.)

         For that same reason, the VC++ 2008 project files are set
         up to build 64-bit libraries and executables *only*.

    It is possible to upgrade these project files to work with newer
    versions of Visual C++, but beware that the upgrade feature tends
    to be problematic.

    If you want to do a 32-bit build on VC++ 2008 or newer, it is
    easiest to open the vc2005\* project files and let Visual Studio
    upgrade them for you.  The alternative, starting with the vc2008
    files, requires that you add a 32-bit build option to all of the
    many targets in MySQL++, then optionally delete the 64-bit targets.
    This is a lot more work.  Plus, it only works if you have the
    64-bit compilers installed, since Visual Studio will refuse to
    open project files where all targets must be built with compilers
    that aren't installed, even if your goal is to immediately adjust
    them to use compilers that *are* installed.

    When converting the VC++ 2008 project files to VC++ 2012, Visual
    Studio will change the output directories from Debug to Debug\x64
    (and similar for Release), but it won't also change the link paths
    from Debug to Debug\x64, so that the library and examples will
    compile but not link.  The migration tool detects that there is
    a problem, but it can't fix its own mess.  You have to manually
    fix it.

    There were also problems in VC++ 2010 when you had converted 32-bit
    VC++ 2008 projects and then were trying to switch them to 64-bit.
    It ended up being simpler in this case to just start over from
    scratch and build your own project files.


Using Nonstandard MySQL Installations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    The Visual Studio project files that come with MySQL++ have
    everything set up correctly for the common case.  The biggest
    assumption in the settings is that you're building against the
    current stable version of Connector/C, which gets installed here
    at the time of this writing:

        C:\Program Files\MySQL\MySQL Connector C 6.1\

    If you installed a different version, or it's in a different
    directory, or you've installed the development files as part of
    MySQL Server on the same machine, you need to change the project
    files to reference the C API development files in that other
    location.  There are two ways to do this.

    The hard way is to make 16 different changes each to 44 separate
    project files.  If you're a talented Visual Studio driver,
    you can do this in as little as about 5 or 6 steps.  You might
    even get it right the first time.  If you are not so talented,
    you have to make all ~700 changes one at a time, and you almost
    certainly will *not* get it right the first time.

    The somewhat easier way is to open all these files in a text
    editor that lets you make a global search and replace on all
    open files.

    The easy way is to install Bakefile (http://bakefile.org/),
    change the value of the MYSQL_WIN_DIR variable near the top of
    mysql++.bkl in the top level of the MySQL++ source tree, and run
    rebake.bat.  This will rebuild all of the project files for you,
    using the new MySQL path in all the many places it's needed.


Building the Library and Example Programs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    You must build both the Debug and Release versions of the library,
    because a release build of your program won't work with a Debug
    version of the MySQL++ DLL.  These DLLs get different names, so
    you can install them in the same directory if needed: mysqlpp_d.dll
    for the Debug version, and mysqlpp.dll for the Release version.

    With the library built, run at least the resetdb and simple1
    examples to ensure that the library is working correctly.
    In addition to the other generic examples, there are a few
    Visual C++ specific examples that you might want to look at in
    examples\vstudio.  See README-examples.txt for further details.

    Once you're sure the library is working correctly, you can run
    the install.hta file at the project root to install the library
    files and headers in a directory of your choosing.
    
    (Aside: You may not have come across the .hta extension before.
    It's for a rarely-used feature of Microsoft's Internet Explorer,
    called HTML Applications.  Know what Adobe AIR is?  Kinda like
    that, only without the compilation into a single binary blob which
    you must install before you can run it.  Just open install.hta
    in a text editor to see how it works.)


Using MySQL++ in Your Own Projects
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    This is covered in the user manual, chapter 9.


Working With Bakefile
~~~~~~~~~~~~~~~~~~~~~
    MySQL++'s top-level Visual Studio project files aren't
    maintained directly.  Instead, we use a tool called Bakefile
    (http://bakefile.org/) to generate them from mysql++.bkl. Since
    there are so many project files in MySQL++, it's often simpler to
    edit this source file and "re-bake" the project files from it than
    to make your changes in Visual Studio.

    To do this, download the native Windows version of Bakefile from the
    web site given above.  Install it, and then put the installation
    directory in your Windows PATH.  Then, open up a command window, cd
    into the MySQL++ directory, and type "rebake".  This will run
    rebake.bat, which rebuilds the Visual Studio project files from
    mysql++.bkl.

    There's more information about using Bakefile in HACKERS.txt.


If You Run Into Problems...
~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Especially if you have linking problems, make sure your project
    settings match the above.  Visual C++ is very picky about things
    like run time library settings.  When in doubt, try running one
    of the example programs.  If it works, the problem is likely in
    your project settings, not in MySQL++.

