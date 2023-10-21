.PRECIOUS: print
DFLAGS=
CFLAGS=-O $(DFLAGS)
LDFLAGS=
PROG	= icom icombg
SRCS	= icom.c icombg.c
OBJS	= icom.o icombg.o
OTHER	= Makefile icom.h
P= | lp
PRINTOPTS = -o-o7 -o-c12

all:	$(PROG)

install: all
	mv icom /usr/local/bin/icom
	mv icombg /usr/local/bin/icombg

clean:
	rm -f $(PROG) $(OBJS)

print:	$(SRCS) $(OTHER)
	pr -f `ls $?` $P $(PRINTOPTS)
	-touch print

icom:	icom.o
	$(CC) icom.o $(LDFLAGS) -lcurses -o icom
icombg:	icombg.o
	$(CC) icombg.o $(LDFLAGS) -o icombg
$(OBJS): icom.h
