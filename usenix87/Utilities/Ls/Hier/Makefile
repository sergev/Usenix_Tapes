# Makefile for hier(1), needed to link with sftw().

hier: hier.c sftw.o
	cc -v -O -s -o hier hier.c sftw.o

sftw.o: sftw.c
	cc -v -O -c sftw.c

debug: hier.c sftw.c
	cc -v -g -o hier hier.c sftw.c
	rm -f hier.o sftw.o	# since they're useless.

test: sftw.c
	cc -v -O -o sftw sftw.c -DTEST

clean:
	rm -f *.o hier sftw core
