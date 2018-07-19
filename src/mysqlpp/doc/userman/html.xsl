<?xml version="1.0" encoding="utf-8"?>

<!-- XSL stylesheet containing additions to the standard DocBook
     chunked-HTML stylesheet.
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		version="1.0">
	
	<!-- Import the standard DocBook stylesheet that this one is based on.
	     We use a web URL, but the local XML catalog should resolve this to
			 the local copy of the stylesheet, if it exists. -->
	<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/chunk.xsl"/>

	<!-- Bring in local changes common to both HTML and FO output -->
	<xsl:include href="common.xsl"/>

	<!-- The DocBook stylesheets use ISO 8859-1 by default, even when the
	     XML files are marked UTF-8.  If you serve such files from a web
			 server that advertises UTF-8 content, browsers display the page
			 incorrectly, because they believe the web server. -->
	<xsl:output method="html" encoding="UTF-8" indent="no"/>

	<!-- HTML-specific XSL parameters -->
	<xsl:param name="chunk.fast" select="0"/>
	<xsl:param name="html.stylesheet" select="'tangentsoft.css'"/>
	<xsl:param name="use.id.as.filename" select="1"/>

	<!-- Special ulink types, to reduce boilerplate link code -->
	<xsl:template match="ulink" name="refman_ulink">
		<xsl:choose>
			<!-- type=mysqlapi: makes hyperlinks to MySQL C API reference manual,
			     given only the function name with dashes instead of underscores
					 as the URL. -->
			<xsl:when test="@type = 'mysqlapi'">
				<tt>
					<a>
						<xsl:variable name="fn_dash" select="@url"/>
						<xsl:variable name="fn_name"
							select="translate($fn_dash, '-', '_')"/>
						<xsl:attribute name="href">
							<xsl:text>http://dev.mysql.com/doc/mysql/en/</xsl:text>
							<xsl:value-of select="$fn_dash"/>
							<xsl:text>.html</xsl:text>
						</xsl:attribute>
						<xsl:value-of select="$fn_name"/>
						<xsl:text>()</xsl:text>
					</a>
				</tt>
			</xsl:when>

			<!-- type=classref: makes hyperlinks to a class in the MySQL++
			     reference manual, given its name. -->
			<xsl:when test="@type = 'classref'">
				<tt>
					<a>
						<xsl:attribute name="href">
							<xsl:text>../refman/classmysqlpp_1_1</xsl:text>
							<xsl:value-of select="@url"/>
							<xsl:text>.html</xsl:text>
						</xsl:attribute>
						<xsl:choose>
							<xsl:when test="count(child::node())=0">
								<xsl:value-of select="@url"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:apply-templates/>
							</xsl:otherwise>
						</xsl:choose>
					</a>
				</tt>
			</xsl:when>

			<!-- type=structref: makes hyperlinks to a struct in the MySQL++
			     reference manual, given its name. -->
			<xsl:when test="@type = 'structref'">
				<tt>
					<a>
						<xsl:attribute name="href">
							<xsl:text>../refman/structmysqlpp_1_1</xsl:text>
							<xsl:value-of select="@url"/>
							<xsl:text>.html</xsl:text>
						</xsl:attribute>
						<xsl:choose>
							<xsl:when test="count(child::node())=0">
								<xsl:value-of select="@url"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:apply-templates/>
							</xsl:otherwise>
						</xsl:choose>
					</a>
				</tt>
			</xsl:when>

			<xsl:otherwise>
				<xsl:call-template name="ulink"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>
