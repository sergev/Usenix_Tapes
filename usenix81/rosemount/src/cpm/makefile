/usr/bin/cpm:	cpm1.o cpm2.o
		cc -n -o /usr/bin/cpm cpm1.o cpm2.o

cpm1.o:		cpm1.c cpm.h
		cc -O -c cpm1.c

cpm2.o:		cpm2.c cpm.h
		cc -O -c cpm2.c
