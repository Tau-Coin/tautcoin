<?xml version="1.0" encoding='UTF-8'?>
<!DOCTYPE sect1 PUBLIC "-//OASIS//DTD DocBook V4.2//EN"
    "http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd">

<sect1 id="threads">
  <title>Using MySQL++ in a Multithreaded Program</title>

  <para>MySQL++ is not &#x201C;thread safe&#x201D; in any
  meaningful sense. MySQL++ contains very little code that
  actively prevents trouble with threads, and all of it is
  optional. We have done some work in MySQL++ to make thread
  safety <emphasis>achievable</emphasis>, but it doesn&#x2019;t come
  for free.</para>

  <para>The main reason for this is that MySQL++ is
  generally I/O-bound, not processor-bound. That is, if
  your program&#x2019;s bottleneck is MySQL++, the ultimate
  cause is usually the I/O overhead of using a client-server
  database. Doubling the number of threads will just let your
  program get back to waiting for I/O twice as fast. Since <ulink
  url="http://www.eecs.berkeley.edu/Pubs/TechRpts/2006/EECS-2006-1.pdf">threads
  are evil</ulink> and generally can&#x2019;t help MySQL++, the only
  optional thread awareness features we turn on in the shipping
  version of MySQL++ are those few that have no practical negative
  consequences. Everything else is up to you, the programmer, to
  evaluate and enable as and when you need it.</para>

  <para>We&#x2019;re going to assume that you are reading this chapter
  because you find yourself needing to use threads for some other
  reason than to speed up MySQL access. Our purpose here is limited
  to setting down the rules for avoiding problems with MySQL++ in a
  multi-threaded program. We won&#x2019;t go into the broader issues of
  thread safety outside the scope of MySQL++. You will need a grounding
  in threads in general to get the full value of this advice.</para>

  <sect2 id="thread-build">
    <title>Build Issues</title>

    <para>Before you can safely use MySQL++ with threads, there are
    several things you must do to get a thread-aware build:</para>

    <orderedlist>
      <listitem>
        <para><emphasis>Build MySQL++ itself with thread awareness
        turned on.</emphasis></para>

        <para>On Linux, Cygwin and Unix (OS X, *BSD, Solaris...),
        pass the <computeroutput>--enable-thread-check</computeroutput>
        flag to the <filename>configure</filename> script. Beware, this
        is only a request to the <filename>configure</filename> script
        to look for thread support on your system, not a requirement
        to do or die: if the script doesn&#x2019;t find what it needs
        to do threading, MySQL++ will just get built without thread
        support. See <filename>README-Unix.txt</filename> for more
        details.</para>

        <para>On Windows, if you use the Visual C++ project files or
        the MinGW Makefile that comes with the MySQL++ distribution,
        threading is always turned on, due to the nature of
        Windows.</para>

        <para>If you build MySQL++ in some other way, such as with
        Dev-Cpp (based on MinGW) you&#x2019;re on your own to enable
        thread awareness.</para>
      </listitem>

      <listitem>
        <para><emphasis>Link your program to a thread-aware build of the
        MySQL C API library.</emphasis></para>

        <para>If you use a binary distribution of MySQL on
        Unixy systems (including Cygwin) you usually get
        two different versions of the MySQL C API library,
        one with thread support and one without. These are
        typically called <filename>libmysqlclient</filename> and
        <filename>libmysqlclient_r</filename>, the latter being the
        thread-safe one. (The &#x201C;<filename>_r</filename>&#x201D;
        means reentrant.)</para>

        <para>If you&#x2019;re using the Windows binary distribution
        of MySQL, you should have only one version of the C
        API library, which should be thread-aware. If you have
        two, you probably just have separate debug and optimized
        builds. See <filename>README-Visual-C++.txt</filename> or
        <filename>README-MinGW.txt</filename> for details.</para>

        <para>If you build MySQL from source, you might only get
        one version of the MySQL C API library, and it can have
        thread awareness or not, depending on your configuration
        choices.</para>
      </listitem>

      <listitem>
        <para><emphasis>Enable threading in your program&#x2019;s build
        options.</emphasis></para>

        <para>This is different for every platform, but it&#x2019;s
        usually the case that you don&#x2019;t get thread-aware builds
        by default. Depending on the platform, you might need to change
        compiler options, linker options, or both. See your development
        environment&#x2019;s documentation, or study how MySQL++ itself
        turns on thread-aware build options when requested.</para>
      </listitem>
    </orderedlist>
  </sect2>


  <sect2 id="thread-conn-mgmt">
    <title>Connection Management</title>

    <para>The MySQL C API underpinning MySQL++ does not allow multiple
    concurrent queries on a single connection. You can run into this
    problem in a single-threaded program, too, which is why we cover the
    details elsewhere, in <xref linkend="concurrentqueries"/>.
    It&#x2019;s a thornier problem when using threads, though.</para>

    <para>The simple fix is to just create a separarate <ulink
    url="Connection" type="classref"/> object for each thread that
    needs to make database queries. This works well if you have a small
    number of threads that need to make queries, and each thread uses
    its connection often enough that the server doesn&#x2019;t <link
    linkend="conn-timeout">time out</link> waiting for queries.</para>

    <para>If you have lots of threads or the frequency of queries is
    low, the connection management overhead will be excessive. To avoid
    that, we created the <ulink url="ConnectionPool" type="classref"/>
    class. It manages a pool of <classname>Connection</classname>
    objects like library books: a thread checks one out, uses
    it, and then returns it to the pool as soon as it&#x2019;s
    done with it. This keeps the number of active connections
    low. We suggest that you keep each connection&#x2019;s
    use limited to a single variable scope for <ulink
    url="http://en.wikipedia.org/wiki/RAII">RAII</ulink> reasons;
    we created a little helper called <ulink url="ScopedConnection"
    type="classref"/> to make that easy.</para>

    <para><classname>ConnectionPool</classname> has three
    methods that you need to override in a subclass to
    make it concrete: <methodname>create()</methodname>,
    <methodname>destroy()</methodname>, and
    <methodname>max_idle_time()</methodname>. These overrides let
    the base class delegate operations it can&#x2019;t successfully do
    itself to its subclass. The <classname>ConnectionPool</classname>
    can&#x2019;t know how to <methodname>create()</methodname>
    the <classname>Connection</classname> objects, because that
    depends on how your program gets login parameters, server
    information, etc.  <classname>ConnectionPool</classname>
    also makes the subclass <methodname>destroy()</methodname>
    the <classname>Connection</classname> objects it created; it
    could assume that they&#x2019;re simply allocated on the heap
    with <methodname>new</methodname>, but it can&#x2019;t be sure,
    so the base class delegates destruction, too. Finally, the base
    class can&#x2019;t know which connection idle timeout policy
    would make the most sense to the client, so it asks its subclass
    via the <methodname>max_idle_time()</methodname> method.</para>

    <para><classname>ConnectionPool</classname> also allows you to
    override <methodname>release()</methodname>, if needed. For simple
    uses, it&#x2019;s not necessary to override this.</para>

    <para>In designing your <classname>ConnectionPool</classname>
    derivative, you might consider making it a <ulink
    url="http://en.wikipedia.org/wiki/Singleton_pattern">Singleton</ulink>,
    since there should only be one pool in a program.</para>

    <para>Another thing you might consider doing is passing a
    <ulink url="ReconnectOption" type="classref"/> object to
    <methodname>Connection::set_option()</methodname> in your
    <methodname>create()</methodname> override before returning the
    new <classname>Connection</classname> pointer. This will cause
    the underlying MySQL C API to try to reconnect to the database
    server if a query fails because the connection was dropped
    by the server. This can happen if the DB server is allowed to
    restart out from under your application. In many applications,
    this isn&#x2019;t allowed, or if it does happen, you might want
    your code to be able to detect it, so MySQL++ doesn&#x2019;t set
    this option for you automatically.</para>

    <para>Here is an example showing how to use connection pools with
    threads:</para>

    <programlisting><xi:include href="cpool.txt" parse="text"
    xmlns:xi="http://www.w3.org/2001/XInclude"/></programlisting>

    <para>The example works with both Windows native
    threads and with POSIX threads.<footnote><para>The file
    <filename>examples/threads.h</filename> contains a few macros and
    such to abstract away the differences between the two threading
    models.</para></footnote> Because thread-enabled builds are only
    the default on Windows, it&#x2019;s quite possible for this program
    to do nothing on other platforms. See above for instructions on
    enabling a thread-aware build.</para>

    <para>If you write your code without checks for thread support
    like you see in the code above and link it to a build of MySQL++
    that isn&#x2019;t thread-aware, it will still try to run. The
    threading mechanisms fall back to a single-threaded mode when
    threads aren&#x2019;t available. A particular danger is that the
    mutex lock mechanism used to keep the pool&#x2019;s internal data
    consistent while multiple threads access it will just quietly
    become a no-op if MySQL++ is built without thread support. We do
    it this way because we don&#x2019;t want to make thread support
    a MySQL++ prerequisite. And, although it would be of limited
    value, this lets you use <classname>ConnectionPool</classname>
    in single-threaded programs.</para>

    <para>You might wonder why we don&#x2019;t just work around
    this weakness in the C API transparently in MySQL++ instead of
    suggesting design guidelines to avoid it. We&#x2019;d like to do
    just that, but how?</para>

    <para>If you consider just the threaded case, you could argue for
    the use of mutexes to protect a connection from trying to execute
    two queries at once. The cure is worse than the disease: it turns a
    design error into a performance sap, as the second thread is blocked
    indefinitely waiting for the connection to free up. Much better to
    let the program get the &#x201C;Commands out of sync&#x201D; error,
    which will guide you to this section of the manual, which tells you
    how to avoid the error with a better design.</para>

    <para>Another option would be to bury
    <classname>ConnectionPool</classname> functionality within MySQL++
    itself, so the library could create new connections at need.
    That&#x2019;s no good because the above example is the most complex
    in MySQL++, so if it were mandatory to use connection pools, the
    whole library would be that much more complex to use. The whole
    point of MySQL++ is to make using the database easier. MySQL++
    offers the connection pool mechanism for those that really need it,
    but an option it must remain.</para>
  </sect2>


  <sect2 id="thread-helpers">
    <title>Helper Functions</title>

    <para><classname>Connection</classname> has several thread-related
    static methods you might care about when using MySQL++ with
    threads.</para>

    <para>You can call
    <methodname>Connection::thread_aware()</methodname> to
    determine whether MySQL++ and the underlying C API library were
    both built to be thread-aware. I want to stress that thread
    <emphasis>awareness</emphasis> is not the same thing as thread
    <emphasis>safety</emphasis>: it&#x2019;s still up to you to make
    your code thread-safe. If this method returns true, it just means
    it&#x2019;s <emphasis>possible</emphasis> to achieve thread-safety,
    not that you actually have it.</para>

    <para>If your program&#x2019;s connection-management strategy
    allows a thread to use a <classname>Connection</classname>
    object that another thread created, you need to know
    about <methodname>Connection::thread_start()</methodname>.
    This function sets up per-thread resources needed to make MySQL
    server calls. You don&#x2019;t need to call it when you use the
    simple <classname>Connection</classname>-per-thread strategy,
    because this function is implicitly called the first time you
    create a <classname>Connection</classname> in a thread. It&#x2019;s
    not harmful to call this function from a thread that previously
    created a <classname>Connection</classname>, just unnecessary. The
    only time it&#x2019;s necessary is when a thread can make calls
    to the database server on a <classname>Connection</classname>
    that another thread created and that thread hasn&#x2019;t already
    created a <classname>Connection</classname> itself.</para>

    <para>If you use <classname>ConnectionPool</classname>, you should
    call <methodname>thread_start()</methodname> at the start of each
    worker thread because you probably can&#x2019;t reliably predict
    whether your <methodname>grab()</methodname> call will create a new
    <classname>Connection</classname> or will return one previously
    returned to the pool from another thread.  It&#x2019;s possible
    to conceive of situations where you can guarantee that each pool
    user always creates a fresh <classname>Connection</classname> the
    first time it calls <methodname>grab()</methodname>, but thread
    programming is complex enough that it&#x2019;s best to take the
    safe path and always call <methodname>thread_start()</methodname>
    early in each worker thread.</para>

    <para>Finally, there&#x2019;s the complementary method,
    <methodname>Connection::thread_end()</methodname>. Strictly
    speaking, it&#x2019;s not <emphasis>necessary</emphasis> to call
    this. The per-thread memory allocated by the C API is small,
    it doesn&#x2019;t grow over time, and a typical thread is going
    to need this memory for its entire run time. Memory debuggers
    aren&#x2019;t smart enough to know all this, though, so they will
    gripe about a memory leak unless you call this from each thread
    that uses MySQL++ before that thread exits.</para>

    <para>Although its name suggests otherwise,
    <methodname>Connection::thread_id()</methodname> has nothing to
    do with anything in this chapter.</para>
  </sect2>


  <sect2 id="thread-data-sharing">
    <title>Sharing MySQL++ Data Structures</title>

    <para>We&#x2019;re in the process of making it safer to share
    MySQL++&#x2019;s data structures across threads. Although things
    are getting better, it&#x2019;s highly doubtful that all problems
    with this are now fixed. By way of illustration, allow me explain
    one aspect of this problem and how we solved it in MySQL++
    3.0.0.</para>

    <para>When you issue a database query that returns rows, you
    also get information about the columns in each row. Since the
    column information is the same for each row in the result set,
    older versions of MySQL++ kept this information in the result
    set object, and each <ulink url="Row" type="classref"/> kept
    a pointer back to the result set object that created it so it
    could access this common data at need. This was fine as long as
    each result set object outlived the <classname>Row</classname>
    objects it returned.  It required uncommon usage patterns to run
    into trouble in this area in a single-threaded program, but in
    a multi-threaded program it was easy. For example, there&#x2019;s
    frequently a desire to let one connection do the queries, and other
    threads process the results.  You can see how avoiding lifetime
    problems here would require a careful locking strategy.</para>

    <para>We got around this in MySQL++ v3.0 by giving these shared data
    structures a lifetime independent of the result set object that
    intitially creates it. These shared data structures stick around
    until the last object needing them gets destroyed.</para>

    <para>Although this is now a solved problem, I bring it up because
    there are likely other similar lifetime and sequencing problems
    waiting to be discovered inside MySQL++. If you would like to
    help us find these, by all means, share data between threads
    willy-nilly.  We welcome your crash reports on the MySQL++
    mailing list. But if you&#x2019;d prefer to avoid problems,
    it&#x2019;s better to keep all data about a query within a single
    thread. Between this and the advice in prior sections, you should
    be able to use threads with MySQL++ without trouble.</para>
  </sect2>
</sect1>
