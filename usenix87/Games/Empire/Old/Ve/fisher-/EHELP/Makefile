INSDIR=..
LIBDIR=../_unix.O
GENLIB=$(LIBDIR)/genlib.a
TERMLIB=-lcurses
HDRDIR=../hdrs
HEADERS= \
	$(HDRDIR)/emp.h \
	$(HDRDIR)/ship.h
CFLAGS= -c -O
SHELL=/bin/sh

ehelp:	$(INSDIR)/ehelp

lint:
	lint -I$(HDRDIR) ehelp.c $(TERMLIB)

$(INSDIR)/ehelp:	_unix.O/ehelp.o $(GENLIB)
	$(CC) -o $(INSDIR)/ehelp _unix.O/ehelp.o $(GENLIB) $(TERMLIB)

_unix.O/ehelp.o:	ehelp.c $(HEADERS) 
	$(CC) $(CFLAGS) -I$(HDRDIR) ehelp.c
	@mv ehelp.o _unix.O/ehelp.o

clean:
	rm -f _unix.O/*.o

clobber:	clean
	rm -f $(INSDIR)/ehelp

FRC:
