#
#	Makefile for 'sun shaded format' recursive tetrahedron
#
BLINN	= #-DBLINN
FAT	= -DFAT
DEBUG	= #-DDEBUG /usr/lib/debug/malloc.o
CFLAGS	= -O $(FAT) $(BLINN) $(DEBUG)
CC	= /bin/cc

tetra: tetra.c
	$(CC) $(CFLAGS) tetra.c -o $@

tetra_sun: tetra.c
	$(CC) $(CFLAGS) -UBLINN tetra.c -o $@

tetra_blinn: tetra.c
	$(CC) $(CFLAGS) -DBLINN tetra.c -o $@
