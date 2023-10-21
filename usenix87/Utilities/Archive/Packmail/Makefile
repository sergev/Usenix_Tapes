CFLAGS= -g

all: unshar packmail

unshar: unshar.c
	cc $(CFLAGS) -o unshar unshar.c

packmail: packmail.c
	cc $(CFLAGS) -o packmail packmail.c

dist: unshar.1 unshar.c packmail.1 packmail.c Makefile packmail header
	packmail -s48000 -t'packmail and unshar' -o'dist' -h'header' \
		Makefile header *.[c1]
	ls -l dist
