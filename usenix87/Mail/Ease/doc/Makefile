#
#	Makefile for Ease document.
#
#	James S. Schoner
#	Purdue University Computing Center
#
# WU changes: use nroff instead of troff; change this to ditroff if you want
#   2/4/87 [tp]  -- note: we lose a little with nroff, but less printing load
TROFF = nroff -Tci300
all:	cover mainbody apen1 apen2 apen3
	
cover: FRC
	$(TROFF) -ms cover

mainbody:
	tbl ease.paper | $(TROFF) -ms

apen1:
	tbl ap1 | $(TROFF) -ms

apen2:
	tbl ap2 | $(TROFF) -ms

apen3:
	tbl ap3 | $(TROFF) -ms

FRC:

