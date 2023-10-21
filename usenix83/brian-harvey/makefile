all:	logo logohead

logo:	y.tab.o logoparse.o zerr.o main.o logoop.o logoaux.o unix.o \
storage.o turtle.o procedit.o logonum.o procvars.o logoproc.o proplist.o
	ld -X -i -o logo /lib/crt0.o *.o -lm -lc

y.tab.c:	logo.y
	yacc logo.y

turtle.c:	atari.i gigi.i adm.i tek.i admtek.i sun.i
	touch turtle.c

logohead:	logohead.c
	cc -O -o logohead logohead.c

help:	splithelp logoman
	./makehelp

splithelp:	splithelp.c
	cc -O -o splithelp splithelp.c

.c.o:	;cc -O -c $*.c

install:
	cp logo /bin/logo
	mkdir /usr/doc/logo
	cp help/* /usr/doc/logo
	cp helpfile applediff olddiff /usr/doc/logo
	mkdir /usr/lib/logo
	cp library/* /usr/lib/logo
	cp logohead /usr/lib/logo

clean:
	rm *.o logo logohead splithelp
