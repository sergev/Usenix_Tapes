CFLAGS=-c
HDRDIR=../hdrs
INSDIR=..
LIBDIR=../_unix.O
EMPGLB=$(LIBDIR)/empglb.o
EMPSUB=$(LIBDIR)/empsub.a

$(INSDIR)/empcre:	_unix.O _unix.O/empcre.o $(EMPGLB) $(EMPSUB)
        $(CC) -o $@ _unix.O/empcre.o $(EMPSUB) $(EMPGLB)

_unix.O:
	mkdir _unix.O

_unix.O/empcre.o:       $(HDRDIR)/empdef.h  empcre.c
        $(CC) $(CFLAGS) -I$(HDRDIR) empcre.c
        @mv empcre.o $@

clobber:
        rm -f $(INSDIR)/empcre
