# Makefile for Trek73.doc

TEXT = header 0.t 1.t 2.t 3.t 4.t 5.t 6.t 7.t 8.t 9.t
PAGE = trek73.6

DOC = trek73.doc
MAN = trek73.man

all: ${DOC} ${MAN}

${DOC}: ${TEXT}
	nroff -ms ${TEXT} > $@

${MAN}: ${PAGE}
	nroff -man ${PAGE} > $@
