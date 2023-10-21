CFLAGS=-c
HDRDIR=../hdrs
INSDIR=../BIN
LIBDIR=../_unix.O
EMPDIS=$(LIBDIR)/empdis.o
EMPCOM=$(LIBDIR)/empcom.a
EMPSUB=$(LIBDIR)/empsub.a
EMPGLB=$(LIBDIR)/empglb.o

IMODS=\
        $(INSDIR)/emp1 \
        $(INSDIR)/emp2 \
        $(INSDIR)/emp3 \
        $(INSDIR)/emp4 \
        $(INSDIR)/emp5 \
        $(INSDIR)/emp7

empn:	$(IMODS)

emp1:	$(INSDIR)/emp1

emp2:	$(INSDIR)/emp2

emp3:	$(INSDIR)/emp3

emp4:	$(INSDIR)/emp4

emp5:	$(INSDIR)/emp5

emp7:	$(INSDIR)/emp7

$(INSDIR)/emp1:	$(EMPDIS) _unix.O/emp1.o $(EMPCOM) $(EMPSUB) $(EMPGLB)
	$(CC) -o $@ $(EMPDIS) _unix.O/emp1.o $(EMPCOM) $(EMPSUB) $(EMPGLB) -lm

$(INSDIR)/emp2:	$(EMPDIS) _unix.O/emp2.o $(EMPCOM) $(EMPSUB) $(EMPGLB)
	$(CC) -o $@ $(EMPDIS) _unix.O/emp2.o $(EMPCOM) $(EMPSUB) $(EMPGLB) -lm

$(INSDIR)/emp3:	$(EMPDIS) _unix.O/emp3.o $(EMPCOM) $(EMPSUB) $(EMPGLB)
	$(CC) -o $@ $(EMPDIS) _unix.O/emp3.o $(EMPCOM) $(EMPSUB) $(EMPGLB) -lm

$(INSDIR)/emp4:	$(EMPDIS) _unix.O/emp4.o $(EMPCOM) $(EMPSUB) $(EMPGLB)
	$(CC) -o $@ $(EMPDIS) _unix.O/emp4.o $(EMPCOM) $(EMPSUB) $(EMPGLB) -lm

$(INSDIR)/emp5:	$(EMPDIS) _unix.O/emp5.o $(EMPCOM) $(EMPSUB) $(EMPGLB)
	$(CC) -o $@ $(EMPDIS) _unix.O/emp5.o $(EMPCOM) $(EMPSUB) $(EMPGLB) -lm

$(INSDIR)/emp7:	$(EMPDIS) _unix.O/emp7.o $(EMPCOM) $(EMPSUB) $(EMPGLB)
	$(CC) -o $@ $(EMPDIS) _unix.O/emp7.o $(EMPCOM) $(EMPSUB) $(EMPGLB) -lm

_unix.O/emp1.o:	emp1.c $(HDRDIR)/empdef.h
	$(CC) $(CFLAGS) -I$(HDRDIR) emp1.c
	@mv emp1.o $@

_unix.O/emp2.o:	emp2.c $(HDRDIR)/empdef.h
	$(CC) $(CFLAGS) -I$(HDRDIR) emp2.c
	@mv emp2.o $@

_unix.O/emp3.o:	emp3.c $(HDRDIR)/empdef.h
	$(CC) $(CFLAGS) -I$(HDRDIR) emp3.c
	@mv emp3.o $@

_unix.O/emp4.o:	emp4.c $(HDRDIR)/empdef.h
	$(CC) $(CFLAGS) -I$(HDRDIR) emp4.c
	@mv emp4.o $@

_unix.O/emp5.o:	emp5.c $(HDRDIR)/empdef.h
	$(CC) $(CFLAGS) -I$(HDRDIR) emp5.c
	@mv emp5.o $@

_unix.O/emp7.o:	emp7.c $(HDRDIR)/empdef.h
	$(CC) $(CFLAGS) -I$(HDRDIR) emp7.c
	@mv emp7.o $@
