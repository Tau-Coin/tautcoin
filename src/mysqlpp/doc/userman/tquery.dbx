<?xml version="1.0" encoding='UTF-8'?>
<!DOCTYPE sect1 PUBLIC "-//OASIS//DTD DocBook V4.2//EN"
    "http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd">

<sect1 id="tquery" xreflabel="Template Queries">
  <title>Template Queries</title>

  <para>Another powerful feature of MySQL++ is being able to set up
  template queries. These are kind of like C&#x2019;s
  <function>printf()</function> facility: you give MySQL++ a string
  containing the fixed parts of the query and placeholders for the
  variable parts, and you can later substitute in values into those
  placeholders.</para>

  <para>The following program demonstrates how to use this feature. This
  is <filename>examples/tquery1.cpp</filename>:</para>

  <programlisting><xi:include href="tquery1.txt" parse="text" 
  xmlns:xi="http://www.w3.org/2001/XInclude"/></programlisting>

  <para>The line just before the call to
  <methodname>query.parse()</methodname> sets the template, and the
  parse call puts it into effect. From that point on, you can re-use
  this query by calling any of several Query member functions that
  accept query template parameters. In this example, we&#x2019;re using
  <methodname>Query::execute()</methodname>.</para>

  <para>Let&#x2019;s dig into this feature a little deeper.</para>


  <sect2 id="tquery-setup">
    <title>Setting up Template Queries</title>

    <para>To set up a template query, you simply insert it into the
    Query object, using numbered placeholders wherever you want to be
    able to change the query. Then, you call the parse() function to
    tell the Query object that the query string is a template query,
    and it needs to parse it:</para>

    <programlisting>
query &lt;&lt; "select (%2:field1, %3:field2) from stock where %1:wheref = %0q:what";
query.parse();</programlisting>

    <para>The format of the placeholder is:</para>

    <programlisting>
%###(modifier)(:name)(:)</programlisting>

    <para>Where &#x201C;###&#x201D; is a number up to three digits. It is
    the order of parameters given to a <ulink type="classref"
    url="SQLQueryParms"/> object, starting from 0.</para>

    <para>&#x201C;modifier&#x201D; can be any one of the following:</para>

    <blockquote>
    <informaltable frame="none">
    <tgroup cols="2">
    <colspec colsep="1" rowsep="1"/>
    <tbody>
      <row>
        <entry><emphasis role="bold">%</emphasis></entry>

        <entry>Print an actual &#x201C;%&#x201D;</entry>
      </row>

      <row>
        <entry><emphasis role="bold">""</emphasis></entry>

        <entry>Don&#x2019;t quote or escape no matter what.</entry>
      </row>

      <row>
        <entry><emphasis role="bold">q</emphasis></entry>

        <entry>This will escape the item using the MySQL C API
        function <ulink url="mysql-escape-string" type="mysqlapi"/>
        and add single quotes around it as necessary, depending on
        the type of the value you use.</entry>
      </row>

      <row>
        <entry><emphasis role="bold">Q</emphasis></entry>

        <entry>Quote but don&#x2019;t escape based on the same rules as
        for &#x201C;q&#x201D;. This can save a bit of processing time if
        you know the strings will never need quoting</entry>
      </row>
    </tbody>
    </tgroup>
    </informaltable>
    </blockquote>

    <para>&#x201C;:name&#x201D; is for an optional name which aids in
    filling SQLQueryParms. Name can contain any alpha-numeric characters
    or the underscore. You can have a trailing colon, which will be
    ignored. If you need to represent an actual colon after the name,
    follow the name with two colons. The first one will end the name and
    the second one won&#x2019;t be processed.</para>
  </sect2>


  <sect2 id="tquery-parms">
    <title>Setting the Parameters at Execution Time</title>

    <para>To specify the parameters when you want to execute a query
    simply use <methodname>Query::store(const SQLString &amp;parm0,
    [..., const SQLString &amp;parm11])</methodname>. This type of
    multiple overload also exists for
    <methodname>Query::storein()</methodname>,
    <methodname>Query::use()</methodname> and
    <methodname>Query::execute()</methodname>. &#x201C;parm0&#x201D;
    corresponds to the first parameter, etc. You may specify up to 25
    parameters. For example:</para>

    <programlisting>
StoreQueryResult res = query.store("Dinner Rolls", "item", "item", "price")</programlisting>

    <para>with the template query provided above would produce:</para>

    <programlisting>
select (item, price) from stock where item = "Dinner Rolls"</programlisting>

    <para>The reason we didn&#x2019;t put the template parameters in
    numeric order...</para>

    <programlisting>
select (%0:field1, %1:field2) from stock where %2:wheref = %3q:what</programlisting>

    <para>...will become apparent shortly.</para>
  </sect2>


  <sect2 id="tquery-defaults">
    <title>Default Parameters</title>

    <para>The template query mechanism allows you to set default
    parameter values. You simply assign a value for the parameter to the
    appropriate position in the
    <varname>Query::template_defaults</varname> array. You can refer to
    the parameters either by position or by name:</para>

    <programlisting>
query.template_defaults[1] = "item";
query.template_defaults["wheref"] = "item";</programlisting>

    <para>Both do the same thing.</para>

    <para>This mechanism works much like C++&#x2019;s default function
    parameter mechanism: if you set defaults for the parameters at the
    end of the list, you can call one of
    <classname>Query</classname>&#x2019;s query execution methods without
    passing all of the values. If the query takes four parameters and
    you&#x2019;ve set defaults for the last three, you can execute the
    query using as little as just one explicit parameter.</para>

    <para>Now you can see why we numbered the template query parameters
    the way we did a few sections earlier. We ordered them so that the
    ones less likely to change have higher numbers, so we don&#x2019;t
    always have to pass them. We can just give them defaults and take
    those defaults when applicable. This is most useful when some
    parameters in a template query vary less often than other
    parameters. For example:</para>

    <programlisting>
query.template_defaults["field1"] = "item"; 
query.template_defaults["field2"] = "price"; 
StoreQueryResult res1 = query.store("Hamburger Buns", "item"); 
StoreQueryResult res2 = query.store(1.25, "price"); </programlisting>

    <para>This stores the result of the following queries in
    <varname>res1</varname> and <varname>res2</varname>,
    respectively:</para>

    <programlisting>
select (item, price) from stock where item = "Hamburger Buns"
select (item, price) from stock where price = 1.25</programlisting>

    <para>Default parameters are useful in this example because we have
    two queries to issue, and parameters 2 and 3 remain the same for
    both, while parameters 0 and 1 vary.</para>

    <para>Some have been tempted into using this mechanism as a way to
    set all of the template parameters in a query:</para>

    <programlisting>
query.template_defaults["what"] = "Hamburger Buns";
query.template_defaults["wheref"] = "item";
query.template_defaults["field1"] = "item"; 
query.template_defaults["field2"] = "price"; 
StoreQueryResult res1 = query.store();</programlisting>

    <para>This can work, but it is <emphasis>not designed to</emphasis>.
    In fact, it&#x2019;s known to fail horribly in one common case. You
    will not get sympathy if you complain on the mailing list about it
    not working. If your code doesn&#x2019;t actively reuse at least one
    of the parameters in subsequent queries, you&#x2019;re abusing
    MySQL++, and it is likely to take its revenge on you.</para>
  </sect2>


  <sect2 id="tquery-errors">
    <title>Error Handling</title>

    <para>If for some reason you did not specify all the parameters when
    executing the query and the remaining parameters do not have their
    values set via <varname>Query::template_defaults</varname>, the
    query object will throw a <ulink type="classref"
    url="BadParamCount"/> object. If this happens, you can get an
    explanation of what happened by calling
    <methodname>BadParamCount::what()</methodname>, like so:</para>

    <programlisting>
query.template_defaults["field1"] = "item"; 
query.template_defaults["field2"] = "price"; 
StoreQueryResult res = query.store(1.25); </programlisting>

    <para>This would throw <classname>BadParamCount</classname> because
    the <varname>wheref</varname> is not specified.</para>

    <para>In theory, this exception should never be thrown. If the
    exception is thrown it probably a logic error in your
    program.</para>
  </sect2>
</sect1>
