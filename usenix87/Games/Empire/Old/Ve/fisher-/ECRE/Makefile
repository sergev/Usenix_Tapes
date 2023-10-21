INSDIR=..
LIBDIR=../_unix.O
GENLIB=$(LIBDIR)/genlib.a
HDRDIR=../hdrs
CFLAGS= -c -O

ecre:	$(INSDIR)/ecre

$(INSDIR)/ecre:	_unix.O/ecre.o $(GENLIB)
	$(CC) -o $(INSDIR)/ecre _unix.O/ecre.o $(GENLIB)

_unix.O/ecre.o:	ecre.c $(HDRDIR)/emp.h 
	$(CC) $(CFLAGS) -I$(HDRDIR) ecre.c
	@mv ecre.o _unix.O/ecre.o

clean:
	rm -f _unix.O/*.o

clobber:	clean
	rm -f $(INSDIR)/ecre

FRC:
