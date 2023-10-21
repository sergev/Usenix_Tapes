CFLAGS=-c
HDRDIR=../hdrs
INSDIR=..
LIBDIR=../_unix.O
EMPGLB=$(LIBDIR)/empglb.o
EMPSUB=$(LIBDIR)/empsub.a

$(INSDIR)/empire:       _unix.O/empire.o $(EMPGLB) $(EMPSUB)
        $(CC) -o $@ _unix.O/empire.o $(EMPSUB) $(EMPGLB)

_unix.O/empire.o:       $(HDRDIR)/empdef.h  empire.c
        $(CC) $(CFLAGS) -I$(HDRDIR) empire.c
        @mv empire.o $@

clobber:
        rm -f $(INSDIR)/empire
