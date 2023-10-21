
sac:		getopt.o sac.c
		cc -O -o sac sac.c getopt.o

getopt.o:	getopt.c
		cc -O -c getopt.c

shar:		
		shar Makefile sac.c getopt.c sac.8 > sac.shar
