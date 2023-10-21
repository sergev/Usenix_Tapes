
# Makefile for systems using modified dbm routines

CFLAGS=-O

all:	sp mksp

sp:	sp.o dbmstuff.o dbm.o misc.o
	cc ${CFLAGS} -o sp sp.o dbmstuff.o misc.o dbm.o

mksp:	mksp.o dbmstuff.o dbm.o misc.o
	cc ${CFLAGS} -o mksp mksp.o dbmstuff.o misc.o dbm.o

calcsoundex:	calcsoundex.c
	cc ${CFLAGS} -o calcsoundex calcsoundex.c

# define BUGFIX if you want the fix included
dbm.o:	dbm.h dbm.c
	cc -c ${CFLAGS} dbm.c

.c.o:
	cc -c ${CFLAGS} $?

clean:
	rm -f sp.o mksp.o dbmstuff.o dbm.o

