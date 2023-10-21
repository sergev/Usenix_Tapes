# Makefile for smail (not a installation makefile)

# @(#)Makefile	1.6   (UUCP-Project/CS)   6/27/86

# If you have compressed files in this dir, uncompress them
# and point UUMAP at the directory with uncompressed files,
# or fiddle with zcat.
UUMAP=/usr/spool/news/maps/mod.map

PATHS	=	/usr/lib/uucp/paths

CFLAGS	=	-O -DPATHS=\"$(PATHS)\"

OBJECTS =	main.o resolve.o deliver.o getopt.o

all: smail binmail

smail:		${OBJECTS}
		cc ${CFLAGS} ${OBJECTS} -o smail

${OBJECTS}:	defs.h

binmail:

install: all
	@echo read INFO.INSTALL

paths:
	pathalias u.Path.* $(UUMAP)/u.* | sort -f > path.out
	cp path.out $(PATHS)

clean:
	rm -f *.o a.out core

clobber: clean
	rm -f smail rmail binmail
