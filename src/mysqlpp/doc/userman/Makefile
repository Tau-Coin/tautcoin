## ------------------------
## Input files
## ------------------------

HTML_DIR=../html/userman
BASENAME=userman
DOCFILE=$(BASENAME).dbx
PDFFILE=../pdf/$(BASENAME).pdf
FOFILE=$(BASENAME).fo
COMMON_SS=common.xsl
FO_SS=fo.xsl
HTML_SS=html.xsl
EX_TXT=cgi_jpeg.txt cpool.txt deadlock.txt fieldinf.txt for_each.txt \
		load_jpeg.txt multiquery.txt resetdb.txt simple1.txt \
		simple2.txt simple3.txt ssqls1.txt ssqls2.txt ssqls3.txt \
		ssqls4.txt ssqls5.txt ssqls6.txt stock.txt store_if.txt \
		tquery1.txt transaction.txt


## ------------------------
## Major output rules
## ------------------------

html: $(EX_TXT) $(HTML_DIR)/index.html

pdf: $(EX_TXT) $(PDFFILE)


## ------------------------
## Standard Makefile targets
## ------------------------

# Notice that this is not the first target in the file, as is standard.
# PDF generation takes longer than HTML generation, so to keep the code-
# test-debug-rebuild cycle short, we generate only the HTML manual by
# default.  You can explicitly say "make pdf" or "make all" when you're
# sure the DocBook file's contents are correct.
all: html pdf

clean:
	@rm -f tags *.fo $(HTML_DIR)/*.html *.log *.out *.pdf $(EX_TXT)


## ------------------------
## How to make output files
## ------------------------

$(PDFFILE): *.dbx *.in $(FO_SS) $(COMMON_SS)
	xsltproc --xinclude $(FO_SS) $(DOCFILE) > $(FOFILE)
	mkdir -p ../pdf
	./fo2pdf $(FOFILE) $(PDFFILE)

$(HTML_DIR)/index.html: *.dbx *.in *.mod *.txt *.xsl
	@xmllint --xinclude --nonet --postvalid --noent --noout $(DOCFILE)
	xsltproc --xinclude --nonet -o $(HTML_DIR)/ $(HTML_SS) $(DOCFILE)


## ------------------------
## Dependency rules
## ------------------------

$(EX_TXT):
	@./mktxt $@

userman.dbx: userman.dbx.in
	( cd ../.. ; ./config.status )
