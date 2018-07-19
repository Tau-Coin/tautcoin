<?xml version="1.0" encoding='UTF-8'?>
<!DOCTYPE sect1 PUBLIC "-//OASIS//DTD DocBook V4.2//EN"
    "http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd">

<sect1 id="overview">
  <title>Overview</title>

  <para>MySQL++ has a lot of complexity and power to cope with the
  variety of ways people use databases, but at bottom it doesn&#x2019;t
  work all that differently than other database access APIs. The usage
  pattern looks like this:</para>

  <orderedlist>
    <listitem><para>Open the connection</para></listitem>

    <listitem><para>Form and execute the query</para></listitem>

    <listitem><para>If successful, iterate through the result
    set</para></listitem>

    <listitem><para>Else, deal with errors</para></listitem>
  </orderedlist>

  <para>Each of these steps corresponds to a MySQL++ class or class
  hierarchy. An overview of each follows.</para>


  <sect2 id="Connection">
    <title>The Connection Object</title>

    <para>A <ulink type="classref" url="Connection"/> object manages the
    connection to the MySQL server. You need at least one of these
    objects to do anything. Because the other MySQL++ objects your
    program will use often depend (at least indirectly) on the
    <classname>Connection</classname> instance, the
    <classname>Connection</classname> object needs to live at least as
    long as all other MySQL++ objects in your program.</para>

    <para>MySQL supports many different types of data connection between
    the client and the server: TCP/IP, Unix domain sockets, and Windows
    named pipes. The generic <classname>Connection</classname> class
    supports all of these, figuring out which one you mean based on the
    parameters you pass to
    <methodname>Connection::connect()</methodname>. But if you know in
    advance that your program only needs one particular connection type,
    there are subclasses with simpler interfaces. For example,
    there&#x2019;s <ulink type="classref" url="TCPConnection"/> if you
    know your program will always use a networked database
    server.</para>
  </sect2>


  <sect2 id="Query">
    <title>The Query Object</title>

    <para>Most often, you create SQL queries using a <ulink
    type="classref" url="Query"/> object created by the
    <classname>Connection</classname> object.</para>

    <para><classname>Query</classname> acts as a standard C++ output
    stream, so you can write data to it like you would to
    <classname>std::cout</classname> or
    <classname>std::ostringstream</classname>. This is the most C++ish
    way MySQL++ provides for building up a query string.  The library
    includes <ulink url="../refman/manip_8h.html">stream
    manipulators</ulink> that are type-aware so it&#x2019;s easy to build
    up syntactically-correct SQL.</para>

    <para><classname>Query</classname> also has a feature called <xref
    linkend="tquery"/> which work something like C&#x2019;s
    <function>printf()</function> function: you set up a fixed query
    string with tags inside that indicate where to insert the variable
    parts. If you have multiple queries that are structurally similar,
    you simply set up one template query, and use that in the various
    locations of your program.</para>

    <para>A third method for building queries is to use
    <classname>Query</classname> with <link
    linkend="ssqls">SSQLS</link>. This feature lets you create C++
    structures that mirror your database schemas. These in turn give
    <classname>Query</classname> the information it needs to build many
    common SQL queries for you. It can <command>INSERT</command>,
    <command>REPLACE</command> and <command>UPDATE</command> rows in a
    table given the data in SSQLS form. It can also generate
    <command>SELECT * FROM SomeTable</command> queries and store the
    results as an STL collection of SSQLSes.</para>
  </sect2>


  <sect2 id="Result">
    <title>Result Sets</title>

    <para>The field data in a result set are stored in a special
    <classname>std::string</classname>-like class called <ulink
    type="classref" url="String"/>. This class has conversion operators
    that let you automatically convert these objects to any of the basic
    C data types. Additionally, MySQL++ defines classes like <ulink
    type="structref" url="DateTime"/>, which you can initialize from a
    MySQL <command>DATETIME</command> string. These automatic
    conversions are protected against bad conversions, and can either
    set a warning flag or throw an exception, depending on how you set
    the library up.</para>

    <para>As for the result sets as a whole, MySQL++ has a number of
    different ways of representing them:</para>

    <sect3 id="SimpleResult">
      <title>Queries That Do Not Return Data</title>

      <para>Not all SQL queries return data. An example is
      <command>CREATE TABLE</command>. For these types of queries, there
      is a special result type (<ulink type="classref"
      url="SimpleResult"/>) that simply reports the state resulting from
      the query: whether the query was successful, how many rows it
      impacted (if any), etc.</para>
    </sect3>

    <sect3 id="StoreQueryResult">
      <title>Queries That Return Data: MySQL++ Data Structures</title>

      <para>The most direct way to retrieve a result set is to use
      <methodname>Query::store()</methodname>. This returns a <ulink
      type="classref" url="StoreQueryResult"/> object, which derives
      from <classname>std::vector&lt;mysqlpp::Row&gt;</classname>,
      making it a random-access container of <ulink type="classref"
      url="Row"/>s. In turn, each <classname>Row</classname> object is
      like a <classname>std::vector</classname> of
      <classname>String</classname> objects, one for each field in the
      result set. Therefore, you can treat
      <classname>StoreQueryResult</classname> as a two-dimensional
      array: you can get the 5th field on the 2nd row by simply saying
      <methodname>result[1][4]</methodname>. You can also access row
      elements by field name, like this:
      <methodname>result[2]["price"]</methodname>.</para>

      <para>A less direct way of working with query results is to use
      <methodname>Query::use()</methodname>, which returns a <ulink
      type="classref" url="UseQueryResult"/> object. This class acts
      like an STL input iterator rather than a
      <classname>std::vector</classname>: you walk through your result
      set processing one row at a time, always going forward. You
      can&#x2019;t seek around in the result set, and you can&#x2019;t
      know how many results are in the set until you find the end. In
      payment for that inconvenience, you get better memory efficiency,
      because the entire result set doesn&#x2019;t need to be stored in
      RAM. This is very useful when you need large result sets.</para>
    </sect3>

    <sect3 id="storein">
      <title>Queries That Return Data: Specialized SQL
      Structures</title>

      <para>Accessing results through MySQL++&#x2019;s data structures is
      a pretty low level of abstraction. It&#x2019;s better than using
      the MySQL C API, but not by much. You can elevate things a little
      closer to the level of the problem space by using the <link
      linkend="ssqls">SSQLS feature</link>. This lets you define C++
      structures that match the table structures in your database
      schema. In addition, it&#x2019;s easy to use SSQLSes with regular
      STL containers (and thus, algorithms) so you don&#x2019;t have to
      deal with the quirks of MySQL++&#x2019;s data structures.</para>

      <para>The advantage of this method is that your program will
      require very little embedded SQL code. You can simply execute a
      query, and receive your results as C++ data structures, which can
      be accessed just as you would any other structure. The results can
      be accessed through the Row object, or you can ask the library to
      dump the results into an STL container &mdash; sequential or
      set-associative, it doesn&#x2019;t matter &mdash; for you. Consider
      this:</para>

      <programlisting>
vector&lt;stock&gt; v;
query &lt;&lt; "SELECT * FROM stock";
query.storein(v);
for (vector&lt;stock&gt;::iterator it = v.begin(); it != v.end(); ++it) {
  cout &lt;&lt; "Price: " &lt;&lt; it-&gt;price &lt;&lt; endl;
}</programlisting>

      <para>Isn&#x2019;t that slick?</para>

      <para>If you don&#x2019;t want to create SSQLSes to match your
      table structures, as of MySQL++ v3 you can now use
      <classname>Row</classname> here instead:</para>

      <programlisting>
vector&lt;mysqlpp::Row&gt; v;
query &lt;&lt; "SELECT * FROM stock";
query.storein(v);
for (vector&lt;mysqlpp::Row&gt;::iterator it = v.begin(); it != v.end(); ++it) {
  cout &lt;&lt; "Price: " &lt;&lt; it->at("price") &lt;&lt; endl;
}</programlisting>

      <para>It lacks a certain syntactic elegance, but it has its
      uses.</para>
    </sect3>
  </sect2>


  <sect2 id="exceptions-intro">
    <title>Exceptions</title>

    <para>By default, the library throws <xref linkend="exceptions"/>
    whenever it encounters an error. You can ask the library to set
    an error flag instead, if you like, but the exceptions carry more
    information. Not only do they include a string member telling you
    why the exception was thrown, there are several exception types,
    so you can distinguish between different error types within a
    single <symbol>try</symbol> block.</para>
  </sect2>
</sect1>
