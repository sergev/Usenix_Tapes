#
#	Makefile for "nav" pre-flight great circle program
#

all: nav

CFLGS=-O

#
# "-lnm" refers to a "new math" library.  You might not have it, but "-lm".
#
nav: main.o nav.o file.o scan.o output.o nav.h
	cc ${CFLGS} main.o nav.o file.o scan.o output.o -o nav -lm

main.o: main.c nav.h

nav.o: nav.c nav.h

file.o: file.c nav.h

scan.o: scan.c nav.h

output.o: output.c

install: nav
	install -c -s nav /whatever/bin/nav

clean:
	-rm -f *.o core

cleanall:
	-rm -f *.o core nav
