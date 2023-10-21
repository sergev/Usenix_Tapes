#
#	Makefile for tcd (Tape color dither)
#
CC	= /bin/cc
CFLAGS	= -O

tcd: tcd.c
	$(CC) $(CFLAGS) -o $@ tcd.c -lpixrect

clean:
	rm -f tcd.o
