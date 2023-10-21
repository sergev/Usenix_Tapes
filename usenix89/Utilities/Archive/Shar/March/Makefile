#
#	Rogue Monster shar makefile
#
#	System V folks, use the following CFLAGS line:
#
#CFLAGS = -O -s -DSYSV
#
#	BSD folks, use the following CFLAGS line:
#
CFLAGS = -O -s

SOURCES = getarg.c shar.c scandir.c
OBJECTS = getarg.o shar.o scandir.o

shar :	$(OBJECTS)
	cc $(CFLAGS) $(OBJECTS) -o shar

getarg.o :	getarg.c
shar.o :	shar.c
scandir.o :	scandir.c
