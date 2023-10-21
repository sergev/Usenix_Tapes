# makefile for rlib	1/4/85

LIBDIR=/usr/lib/news
SPOOLDIR=/usr/usr/news
BINDIR=/usr/bin
NEWSUSR=news
NEWSGRP=news
CPU=other
USGREL=50
BSDREL=6
TERMCAP=-lcurses

CFLAGS = $(DEBUG)
DEBUG = -O
MAKE = make

RLIB = aflock.o afopen.o cancel.o cgtdate.o dir.o dirname.o\
genafopen.o getadate.o getdate.o ftime.o gethead.o getopt.o\
getuser.o hfgets.o hfree.o hxchg.o isadmin.o isre.o launder.o\
lookup.o ngchain.o ngmatch.o openrc.o pgetuser.o read.o\
replyname.o rewinddir.o rmnf.o readinrc.o roptions.o titmat.o\
write.o setupgrp.o addrc.o allgroups.o findgroup.o gfopen.o\
makehimask.o nextgrp.o prevgrp.o bfr.o bcopy.o bzero.o ckfopen.o\
getaddr.o hash.o lcase.o nsavestr.o nstrip.o prefix.o process.o\
rename.o savestr.o ckmalloc.o scopyn.o strpbrk.o strcspn.o\
strspn.o strtok.o tomsgid.o


all: setuptime makefile rlib.a cgtdate rpathinit.o newer

install: all
	/bin/cp cgtdate $(LIBDIR)/cgtdate

setuptime: setup config
	sh setup

makefile: genmakefile makedefs
	genmakefile	
	@echo 'Makefile changed.  Restart make program.'
	@sh -c 'exit 22'

rlib.a: $(RLIB)
	rm -f $@
	ar rc $@ $(RLIB)

rpathinit.o: mypathinit.c defs.h newsdefs.h $(FRC)
	$(CC) -c $(CFLAGS) mypathinit.c
	mv mypathinit.o rpathinit.o

genafopen.o lock.o lookup.o ngchain.o read.o: af.h artfile.h
readinrc.o setupgrp.o: artfile.h
addrc.o nextgrp.o prevgrp.o readinrc.o setupgrp.o: newsrc.h
allgroups.o readinrc.o: ng.h
cancel.o gethead.o hfree.o hxchg.o: arthead.h
$(RLIB): $(FRC)

cgtdate: cgtdatecmd.o cgtdate.o getadate.o getdate.o
	$(CC) $(LFLAGS) -o $@ cgtdatecmd.o rlib.a

newer: newer.c
	$(CC) -o $@ newer.c

FRC:
