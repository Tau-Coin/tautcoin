<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE sect1 PUBLIC "-//OASIS//DTD DocBook V4.2//EN"
    "http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd">

<sect1 id="intro">
  <title>Introduction</title>

  <para>MySQL++ is a powerful C++ wrapper for MySQL&#x2019;s
  C API<footnote><para>The MySQL C API is also known as <ulink
  url="https://dev.mysql.com/downloads/connector/c/">Connector/C</ulink>.</para></footnote>.
  Its purpose is to make working with queries as easy as working with
  STL containers.</para>

  <para>The latest version of MySQL++ can be found at <ulink
  url="http://tangentsoft.net/mysql++/">the official web
  site</ulink>.</para>

  <para>Support for MySQL++ can be had on <ulink
  url="http://lists.mysql.com/plusplus">the mailing list</ulink>. That
  page hosts the mailing list archives, and tells you how you can
  subscribe.</para>


  <sect2 id="history">
    <title>A Brief History of MySQL++</title>

    <para>MySQL++ was created in 1998 by Kevin Atkinson. It started
    out MySQL-specific, but there were early efforts to try and
    make it database-independent, and call it SQL++. This is where
    the old library name &#x201C;sqlplus&#x201D; came from. This
    is also why the old versions prefixed some class names with
    &#x201C;Mysql&#x201D; but not others: the others were supposed to
    be the database-independent parts. All of Kevin&#x2019;s releases
    had pre-1.0 version numbers.</para>

    <para>Then in 1999, <ulink url="http://www.mysql.com/">MySQL
    AB</ulink> took over development of the library. In the beginning,
    <ulink url="http://en.wikipedia.org/wiki/Monty_Widenius">Monty
    Widenius</ulink> himself did some of the work, but later gave it
    over to another MySQL employee, Sinisa Milivojevic. MySQL released
    versions 1.0 and 1.1, and then Kevin gave over maintenance to
    Sinisa officially with 1.2, and ceased to have any involvement
    with the library&#x2019;s maintenance. Sinisa went on to maintain
    the library through 1.7.9, released in mid-2001. It seems to be
    during this time that the dream of multiple-database compatibility
    died, for obvious reasons.</para>

    <para>With version 1.7.9, MySQL++ went into a period of
    stasis, lasting over three years. (Perhaps it was the
    ennui and retrenchment following the collapse of <ulink
    url="http://en.wikipedia.org/wiki/Dot-com_bubble">the
    bubble</ulink> that caused them to lose interest.) During this
    time, Sinisa ran the MySQL++ mailing list and supported its users,
    but made no new releases. Contributed patches were either ignored
    or put up on the MySQL++ web site for users to try, without any
    official blessing.</para>

    <para>The biggest barrier to using MySQL++ during this period
    is that the popular C++ compilers of 2001 weren&#x2019;t all
    that compatible with the C++ Standard. As a result, MySQL++
    used many nonstandard constructs, to allow for compatibility
    with older compilers. Each new compiler released in the
    following years increased compliance, either warning
    about or rejecting code using pre-Standard constructs.
    In particular, <ulink url="http://gcc.gnu.org/">GCC</ulink>
    was emerging from the mess following the <ulink
    url="http://en.wikipedia.org/wiki/GNU_Compiler_Collection#EGCS">EGCS
    fork</ulink> during this time. The fork was healed officially
    in 1999, but there&#x2019;s always a delay of a few years between
    the release of a new GCC and widespread adoption. The post-EGCS
    versions of GCC were only beginning to become popular by 2001,
    when development on MySQL++ halted. As a result, it became
    increasingly difficult to get MySQL++ to build cleanly as newer
    compilers came out. Since MySQL++ uses templates heavily, this
    affected end user programs as well: MySQL++ code got included
    directly in your program, so any warnings or errors it caused
    became your program&#x2019;s problem.</para>

    <para>As a result, most of the patches contributed to the MySQL++
    project during this period were to fix up standards compliance
    issues. Because no one was bothering to officially test and bless
    these patches, you ended up with the worst aspects of a <ulink
    url="http://en.wikipedia.org/wiki/The_Cathedral_and_the_Bazaar">bazaar</ulink>
    development model: complete freedom of development, but no guiding
    hand to select from the good stuff and reject the rest. Many of the
    patches were mutually incompatible. Some would build upon other
    patches, so you had to apply them in the proper sequence. Others
    did useful things, but didn&#x2019;t give a fully functional copy of
    MySQL++. Figuring out which patch(es) to use was an increasingly
    frustrating exercise as the years wore on, and newer GCCs became
    popular.</para>

    <para>In early August of 2004, Warren Young got fed up with this
    situation and took over. He released 1.7.10 later that month,
    which did little more than make the code build with GCC 3.3 without
    warnings. Since then, with a little help from his friends on the
    Net, MySQL++ has lost a lot of bugs, gained a lot of features,
    gained a few more bugs, lost them again... MySQL++ is alive and
    healthy now.</para>
  </sect2>


  <sect2 id="asking-questions">
    <title>If You Have Questions...</title>

    <para>If you want to email someone to ask questions about
    this library, we greatly prefer that you send mail to the
    <ulink url="http://lists.mysql.com/plusplus">MySQL++ mailing
    list</ulink>. The mailing list is archived, so if you have
    questions, do a search to see if the question has been asked
    before.</para>

    <para>You may find people&#x2019;s individual email addresses in various
    files within the MySQL++ distribution. Please do not send mail
    to them unless you are sending something that is inherently
    personal. Not all of the principal developers of MySQL++ are still
    active in its development; those who have dropped out have no wish
    to be bugged about MySQL++. Those of us still active in MySQL++
    development monitor the mailing list, so you aren&#x2019;t getting any
    extra &#x201C;coverage&#x201D; by sending messages to additional
    email addresses.</para>
  </sect2>
</sect1>
