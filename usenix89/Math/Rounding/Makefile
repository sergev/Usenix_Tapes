#
# Makefile for testr2.c [test program for round2() function]
#

DESTDIR = .

INCDIR = .

LIBDIR = .

FNAME = testr2

CSRC = round2.c testr2.c

TMPFILES = round2.o testr2.o

COBJCTS = round2.o testr2.o

CFLAGS =  -O -DR2DEBUG

LIBS = -lm

LINTLIB = 

PRTFILES = $(CSRC) round2.misc Makefile

PRTNAME = $(FNAME)

FFHDR = "@          `date`	`pwd`/*	Page @@@"

PRTFMT = /local/bin/ff -b -N 8 -H 3 -h $(FFHDR) -T 8 -w 128 -i 2

PRTDEV = /usr/ucb/lpr -Plp -J$(PRTNAME)

CC = cc

.DEFAULT:
	co $@

all: testr2
	@echo Done.

testr2: $(COBJCTS)
	$(CC) -o $(FNAME) $(COBJCTS) $(LIBS)

round2.o: round2.c

testr2.o: testr2.c

install: all
	cp $(FNAME) $(DESTDIR)/$(FNAME)
	chmod 755 $(DESTDIR)/$(FNAME)
	strip $(DESTDIR)/$(FNAME)

clean:
	rm -f $(TMPFILES) core make.out

clobber: clean
	 rm -f $(FNAME)

lint:
	lint -ac -I$(INCDIR) $(CSRC) $(LINTLIB)

print:
	$(PRTFMT) $(PRTFILES) | $(PRTDEV)

