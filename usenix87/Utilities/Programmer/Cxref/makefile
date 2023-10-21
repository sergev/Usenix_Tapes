# makefile for cxref -- C cross referencing program
#
# Arnold Robbins, Information and Computer Science, Georgia Tech
#	gatech!arnold
# Copyright (c) 1984 by Arnold Robbins.
# All rights reserved.
# This program may not be sold, but may be distributed
# provided this header is included.

# some files are system dependant, e.g. where sort is.
# change the appropriate macro definitions and recompile.


### definitions of files to compile and load, and other targets for make

SCANOBJS= docxref.o cscan.o
SCANSRCS= docxref.c cscan.l

CXREF = cxref
INCLS= constdefs.h basename.c
PROGS= docxref fmtxref cxrfilt $(CXREF)
SRCS=  $(SCANSRCS) fmtxref.c cxrfilt.c $(CXREF).c
DOCS=  README makefile cxref.1

PRINTS= $(INCLS) $(SRCS) $(DOCS)

CFLAGS= -O -Dindex=strchr -Drindex=strrchr

### system dependant definitions, change when you install cxref

# for my use during development, put in my bin, but see next few lines.
DESTDIR= $(BIN)
LIB= $(BIN)

# when installing, use the lines below; change DESTDIR to local bin of choice.
# DESTDIR=/ics/bin
# LIB=/usr/local/lib/cxref

# where to put the man page, use 1 instead of l if you don't have a manl.
MANSEC=l

# lex library, may be -lln on some systems
LEXLIB= -ll

# may be /bin/sort on some systems
SORT=/bin/sort

# printer program, prt is for me, use pr on other systems
P=prt

# who will own the thing.  admin on our system, may be "bin" or something
# else on your system
ADMIN= bin

all: $(PROGS)
	@echo "	all" done

docxref: $(SCANOBJS)
	$(CC) $(SCANOBJS) $(LEXLIB) -o $@

cscan.o docxref.o cxrfilt.o: $(INCLS)

fmtxref: fmtxref.c
	$(CC) $(CFLAGS) $@.c $(LDFLAGS) -o $@

cxrfilt: cxrfilt.c
	$(CC) $(CFLAGS) $@.c $(LDFLAGS) -o $@

$(CXREF): $(CXREF).c
	$(CC) $(CFLAGS) -DSRCDIR='"$(LIB)"' -DSORT='"$(SORT)"' $@.c $(LDFLAGS) -o $@

print:
	$(P) $(PRINTS) | lpr -b 'Cxref New Source'
	touch print2

print2: $(PRINTS)
	$(P) $? | lpr -b 'Cxref New Source'
	touch print2

### edit this before installing

install: $(PROGS)
# don't remove my bin!
#	rm -fr $(LIB)
	rm -f $(DESTDIR)/cxref
#	mkdir $(LIB)
	cp $(CXREF) $(DESTDIR)/$(CXREF)
	cp docxref  $(LIB)/docxref
	cp fmtxref  $(LIB)/fmtxref
	cp cxrfilt $(LIB)/cxrfilt
#	cp cxref.1 /usr/man/man$(MANSEC)/cxref.$(MANSEC)
#	cd $(DESTDIR); chmod 711 cxref; chown $(ADMIN) cxref; chgrp $(ADMIN) cxref
#	cd $(LIB); chmod 711 docxref fmtxref cxrfilt
#	cd $(LIB); chown $(ADMIN) docxref fmtxref cxrfilt
#	cd $(LIB); chgrp $(ADMIN) docxref fmtxref cxrfilt

clean:
	rm -f $(SCANOBJS)

clobber: clean
	rm -f $(PROGS) print2
