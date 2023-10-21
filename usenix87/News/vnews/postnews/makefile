# makefile 2.11-B 1/12/85

LIB=../lib
LIBDIR=/usr/lib/news
SPOOLDIR=/usr/usr/news
BINDIR=/usr/bin
NEWSUSR=news
NEWSGRP=news
CPU=other
USGREL=50
BSDREL=6
TERMCAP=-lcurses

POSTNEWS = postnews.o artlist.o $(LIB)/rpathinit.o $(LIB)/rlib.a
POSTREPLY = postreply.o $(LIB)/rpathinit.o $(LIB)/rlib.a
POSTNM = postnm1.o postnm2.o postnm3.o postnm4.o postnm5.o $(LIB)/rpathinit.o $(LIB)/rlib.a
RECMAIL = recmail.o
CFLAGS = $(DEBUG) -I$(LIB)
DEBUG = -O

all: makefile postnews postreply postnm distributions gparent.wm

makefile: genmakefile $(LIB)/makedefs
	genmakefile
	@echo 'Makefile changed, so restart make.'
	@sh -c 'exit 22'

install: all $(LIBDIR)/distributions $(LIBDIR)/moderators
	-/bin/mv $(BINDIR)/postnews $(BINDIR)/opostnews
	/bin/cp postnews postnm gparent $(BINDIR)
	/bin/cp postreply $(LIBDIR)/postreply

postnews: $(POSTNEWS)
	$(CC) $(DEBUG) -o $@ $(POSTNEWS)

postreply: $(POSTREPLY)
	$(CC) $(DEBUG) -o $@ $(POSTREPLY)

postnm: $(POSTNM)
	$(CC) $(DEBUG) -o $@ $(POSTNM)

recmail: $(RECMAIL)
	$(CC) $(DEBUG) -o $@ $(RECMAIL)

distributions: distlist
	makedist > distributions

gparent.wm: gparent.wm.c
	$(CC) gparent.wm.c
	a.out > gparent.wm
	rm a.out

$(LIBDIR)/distributions: distributions
	-/bin/mv $(LIBDIR)/distributions $(LIBDIR)/odistributions
	/bin/cp distributions $(LIBDIR)/distributions

$(LIBDIR)/moderators:
	/bin/cp moderators $@

postnews.o postreply.o recmail.o: $(FRC)
postnm1.o postnm2.o postnm3.o postnm4.o postnm5.o: postnm.h $(FRC)

FRC:
