CFLAGS = -c
HDRDIR=../hdrs
INSDIR=..
LIBDIR=../_unix.O
GENLIB=$(LIBDIR)/genlib.a
UPDLIB=$(LIBDIR)/updlib.a

eupd:	$(INSDIR)/eupd

$(INSDIR)/eupd:	_unix.O/y.tab.o $(UPDLIB) $(GENLIB)
	$(CC) -o $@ _unix.O/y.tab.o $(UPDLIB) $(GENLIB) -ll

_unix.O/y.tab.o:	y.tab.c lex.yy.c $(HDRDIR)/emp.h $(HDRDIR)/ship.h
	$(CC) $(CFLAGS) -I$(HDRDIR) y.tab.c
	@mv y.tab.o $@

y.tab.c:	eupd.y
	yacc eupd.y

lex.yy.c:	eupd.l
	lex -n eupd.l

clean:
	rm -f lex.yy.c y.tab.c _unix.O/y.tab.o

clobber: clean
	rm -f $(INSDIR)/eupd
