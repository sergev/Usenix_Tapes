CFLAGS=-c
HDRDIR=../hdrs
INSDIR=../BIN
LIBDIR=../_unix.O
EMPGLB=$(LIBDIR)/empglb.o
EMPSUB=$(LIBDIR)/empsub.a
LIB=_unix.O/fixlib.a
HDRDEP= \
        $(LIB)(nations.o) \
        $(LIB)(sectors.o) \
        $(LIB)(ships.o) \
        $(LIB)(treats.o) \
        $(LIB)(loans.o)

$(INSDIR)/empfix:       _unix.O/main.o _unix.O/fixlib.a $(EMPGLB) $(EMPSUB)
        $(CC) -o $(INSDIR)/empfix _unix.O/main.o _unix.O/fixlib.a $(EMPSUB) $(EM
PGLB)

empfix: $(INSDIR)/empfix

_unix.O/main.o: $(HDRDIR)/empdef.h main.c
        $(CC) $(CFLAGS) -I$(HDRDIR) main.c
        @mv main.o _unix.O/main.o

$(LIB): \
        $(LIB)(bytefix.o) \
        $(LIB)(chfix.o) \
        $(LIB)(fixup.o) \
        $(LIB)(floatfi.o) \
        $(LIB)(loans.o) \
        $(LIB)(longfix.o) \
        $(LIB)(nations.o) \
        $(LIB)(sectors.o) \
        $(LIB)(ships.o) \
        $(LIB)(treats.o) \
        $(LIB)(wordfix.o)
        $(CC) $(CFLAGS) -I$(HDRDIR) $(?:.o=.c)
        @if [ -s $(LIB) ];\
        then\
                ar r $(LIB) $?;\
        else\
                ar cq $(LIB) $?;\
        fi
        @rm -f $?
.c.a:;

$(HDRDEP):      $(HDRDIR)/empdef.h

clobber:
        rm -f $(INSDIR)/empfix
