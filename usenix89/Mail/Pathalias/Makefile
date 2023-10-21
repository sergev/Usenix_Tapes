#!/bin/make -f
# pathalias -- by steve bellovin, as told to peter honeyman

# if you can't or don't intend to use dbm files, don't bother with makedb
# DBM = -ldbm
# or if you roll your own ...
# DBM = dbm.o

CC = cc
CFLAGS = -O
# As of 9/86 you can do the whole map with split i/d.  If you don't use
# split i/d, it may still work.  Look at addnode.c's hash table size;
# you'll have to make the hash table smaller to get it to fit.
LDFLAGS = -i
YFLAGS = -d

OBJ =	addlink.o addnode.o extern.o local.o main.o mapit.o mapaux.o \
	mem.o parse.o putnode.o /usr/local/lib/getopt.o
HDRS = def.h config.h
CSRC = addlink.c addnode.c extern.c local.c main.c mapit.c mapaux.c mem.c putnode.c
SRC = $(CSRC) parse.y
LSRC = $(CSRC) parse.c

all: pathalias printit makedb

pathalias: $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o pathalias

printit: printit.o putnode.o config.h def.h extern.o mem.o
	$(CC) $(LDFLAGS) -o printit printit.o putnode.o extern.o mem.o

$(OBJ):	$(HDRS)
printit.o: $(HDRS)

parse.c: parse.y $(HDRS)
	$(YACC) $(YFLAGS) parse.y
	mv y.tab.c parse.c

makedb: makedb.o
	$(CC) $(LDFLAGS) makedb.o $(DBM) -o makedb

makedb.o: config.h

clean:
	rm -f *.o y.tab.? parse.c makedb pathalias printit

clobber: clean
	rm -f pathalias makedb

tags: $(SRC) $(HDRS) makedb.c
	ctags -w $(HDRS) $(SRC)

bundle:
	shar README CHANGES pathalias.1 Makefile ${HDRS} >Shar.1
	shar ${SRC} makedb.c >Shar.2

lint:	$(LSRC)
	lint $(CFLAGS) $(LSRC)
	lint makedb.c

install: pathalias printit makedb
	strip pathalias printit makedb
	alias news cp pathalias printit makedb /usr/spool/news/lib

