# PROGS is what to make, DESTDIR is where to install it
R=/usr/local
DESTDIR=$R/usr/bin
DOCDIR=
LIBDESTDIR=$R/usr/lib/troff
MANDIR=$R/usr/man
PROGS=mkpat extract
MAKE=make
CLEAN=clean

# HACKS are common lint/cc/f77 flags: -D,-I,-U
# R is an optional root (ie. prefix to DESTDIR)

HACKS=
CFLAGS=-O -s $(HACKS)
LOADLIBES=
LINT=lint
LINTFLAGS=-phxabc $(HACKS)
CHMODFLAGS=u=rwx,og=rx
SHELL=/bin/sh

all:    $(PROGS)                                 # default

cp install:     $(PROGS)
	-mkdir $(LIBDESTDIR) 2>/dev/null
	cp extract $(DESTDIR)
	cp extract.1 $(MANDIR)/man1
	cp mkpat $(LIBDESTDIR)
	$(MAKE) $(CLEAN)

clean:
	rm -f $(PROGS) *.o core y.tab.c lex.yy.c a.out

lint:
	$(LINT) $(LINTFLAGS) *.c

$(DESTDIR)/extract: extract.sh

$(LIBDESTDIR)/mkpat: mkpat.c
