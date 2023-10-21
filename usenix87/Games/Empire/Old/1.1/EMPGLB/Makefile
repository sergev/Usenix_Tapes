CFLAGS=-c
HDRDIR=../hdrs
INSDIR=../_unix.O
EMPGLB=$(INSDIR)/empglb.o

$(EMPGLB):      $(HDRDIR)/empdef.h  empglb.c
        $(CC) $(CFLAGS) -I$(HDRDIR) empglb.c
        mv empglb.o $@

clobber:
        rm -f $(EMPGLB)
