# Makefile for 'ulimit'
# Copyright (C) 1987 by Paul Sutcliffe, Jr.  (devon!paul)

CC	= cc -Dvoid=int
NROFF	= nroff
CFLAGS	= -s -O
DESTDIR	= /usr/local/bin
ROOTUSR	= root

ulimit:	ulimit.c
	$(CC) $(CFLAGS) -o ulimit ulimit.c

install: ulimit
	cp ulimit $(DESTDIR)
	chown $(ROOTUSR) $(DESTDIR)/ulimit
	chgrp bin $(DESTDIR)/ulimit
	chmod 4711 $(DESTDIR)/ulimit

man:	ulimit.1
	$(NROFF) -man ulimit.1

clean:
	rm -f ulimit.o core

clobber: clean
	rm -f ulimit

