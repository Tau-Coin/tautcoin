<?xml version="1.0" encoding="utf-8"?>

<!-- XSL stylesheet containing things common to both HTML and FO
     transformations. -->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:param name="section.autolabel" select="1"/>
  <xsl:param name="section.autolabel.max.depth" select="2"/>
  <xsl:param name="toc.section.depth" select="2"/>

  <!-- Hack to let us get <mathphrase>-like behavior in DB 4.2 -->
  <!-- From: http://www.sagehill.net/docbookxsl/Math.html -->
  <xsl:template match="phrase[@role = 'math']">
    <xsl:call-template name="inline.italicseq"/>
  </xsl:template>
</xsl:stylesheet>

