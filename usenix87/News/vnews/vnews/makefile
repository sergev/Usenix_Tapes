# Vnews makefile 2.11-B 1/18/85

# definitions

LIBDIR=/usr/lib/news
SPOOLDIR=/usr/usr/news
BINDIR=/usr/bin
NEWSUSR=news
NEWSGRP=news
CPU=other
USGREL=50
BSDREL=6
TERMCAP=-lcurses
LOGDIR =

IBMFLAGS =
DEBUG = -O
CFLAGS = $(DEBUG) $(IBMFLAGS) -I../lib
LFLAGS = -i $(DEBUG) $(IBMFLAGS)

VOBJECTS = readnews.o vextern.o $(LOGDIR) vreadr.o dispcntl.o artseq.o termio.o virtterm.o

# dependencies

all: makefile vnews

makefile: genmakefile ../lib/makedefs
	genmakefile
	@echo 'Makefile changed, so restart make.'
	@sh -c 'exit 22'

install: all
	-/bin/mv $(BINDIR)/vnews $(BINDIR)/ovnews
	/bin/cp vnews $(BINDIR)/vnews
	-/bin/mv $(LIBDIR)/vnews.help $(LIBDIR)/ovnews.help
	/bin/cp vnews.help $(LIBDIR)/vnews.help

clean:
	rm -f core *.o

readnews.o:  readnews.c rparams.h
	$(CC) $(CFLAGS) -c readnews.c

rfuncs.o:  rfuncs.c rparams.h
	$(CC) $(CFLAGS) -c rfuncs.c

logdir.o: logdir.c
	$(CC) $(CFLAGS) -c logdir.c

vnews:	$(VOBJECTS) ../lib/rpathinit.o ../lib/rlib.a
	$(CC) $(LFLAGS) $(VOBJECTS) ../lib/rpathinit.o ../lib/rlib.a $(TERMCAP) -o $@

vreadr.o dispcntl.o artseq.o termio.o: vnews.h rparams.h

vextern.o:  vextern.c rparams.h

VNEWSLINT = artseq.c dispcntl.c readnews.c vextern.c rfuncs.c\
termio.c virtterm.c vreadr.c\
../lib/addrc.c ../lib/afopen.c ../lib/allgroups.c ../lib/bcopy.s\
../lib/bfr.c ../lib/bzero.c ../lib/cancel.c ../lib/ckfopen.c\
../lib/ckmalloc.c ../lib/dirname.c ../lib/findgroup.c\
../lib/genafopen.c ../lib/gethead.c ../lib/getuser.c ../lib/gfopen.c\
../lib/hash.c ../lib/hfgets.c ../lib/isadmin.c ../lib/lookup.c\
../lib/makehimask.c ../lib/mypathinit.c ../lib/nextgrp.c\
../lib/ngchain.c ../lib/ngmatch.c ../lib/nsavestr.c ../lib/nstrip.c\
../lib/openrc.c ../lib/pgetuser.c ../lib/prefix.c ../lib/prevgrp.c\
../lib/process.c ../lib/read.c ../lib/readinrc.c ../lib/rename.c\
../lib/roptions.c ../lib/savestr.c ../lib/scopyn.c ../lib/setupgrp.c\
../lib/titmat.c

vnewslint:
	lint -I../lib -DSPOOLDIR=\"$(SPOOLDIR) -DLIBDIR=\"$(LIBDIR) $(VNEWSLINT)		# You must be masochistic
