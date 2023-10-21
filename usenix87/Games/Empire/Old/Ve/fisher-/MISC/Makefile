INSDIR=..
LIBDIR=../_unix.O
GENLIB=$(LIBDIR)/genlib.a
HDRDIR=../hdrs
CFLAGS= -c -O

all:	pworld rdship
	
pworld:	$(INSDIR)/pworld

$(INSDIR)/pworld:	_unix.O/pworld.o $(GENLIB)
	$(CC) -o $(INSDIR)/pworld _unix.O/pworld.o $(GENLIB)

_unix.O/pworld.o:	pworld.c $(HDRDIR)/emp.h 
	$(CC) $(CFLAGS) -I$(HDRDIR) pworld.c
	@mv pworld.o _unix.O/pworld.o

rdship:	$(INSDIR)/rdship

$(INSDIR)/rdship:	_unix.O/rdship.o $(GENLIB)
	$(CC) -o $(INSDIR)/rdship _unix.O/rdship.o $(GENLIB)

_unix.O/rdship.o:	rdship.c $(HDRDIR)/ship.h 
	$(CC) $(CFLAGS) -I$(HDRDIR) rdship.c
	@mv rdship.o _unix.O/rdship.o

clean:
	rm -f _unix.O/*.o

clobber:	clean
	rm -f $(INSDIR)/pworld $(INSDIR)/rdship

FRC:
