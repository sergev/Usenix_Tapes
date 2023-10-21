CFLAGS= -c -O
HDRDIR =../hdrs
INSDIR=../_unix.O
LIB=$(INSDIR)/genlib.a

$(LIB): \
	$(LIB)(getopt.o) \
	$(LIB)(strchr.o) \
	$(LIB)(strspn.o) \
	$(LIB)(secno.o) \
	$(LIB)(getsec.o) \
	$(LIB)(getship.o)
	$(CC) $(CFLAGS) -I$(HDRDIR) $(?:.o=.c)
	@ar r $(LIB) *.o
	@rm -f *.o
	@echo $@ is now up to date
.c.a:;

$(LIB)(secno.o):	$(HDRDIR)/emp.h

$(LIB)(getsec.o):	$(HDRDIR)/emp.h

$(LIB)(getship.o):	$(HDRDIR)/emp.h $(HDRDIR)/ship.h

clean:

clobber:
	rm -f $(LIB)

.PRECIOUS:	$(LIB)

