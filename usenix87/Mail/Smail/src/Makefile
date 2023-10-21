# Makefile for smail (not a installation makefile)

# @(#)Makefile	2.2 (smail) 1/16/87

CFLAGS	=	-O
#
# System V Release 2.0 sites can use -lmalloc for a faster malloc
#
#LIBS	=	-lmalloc

OBJECTS =	main.o map.o resolve.o deliver.o misc.o alias.o headers.o getpath.o getopt.o

all: smail svbinmail lcasep pathproc

smail:		$(OBJECTS)
		cc $(CFLAGS) $(OBJECTS) -o smail $(LIBS)

$(OBJECTS):	defs.h
		cc $(CFLAGS) -c $<

svbinmail:	svbinmail.c defs.h
		cc $(CFLAGS) svbinmail.c -o svbinmail

lcasep:		lcasep.c
		cc $(CFLAGS) lcasep.c -o lcasep

pathproc: pathproc.sh
	cp pathproc.sh pathproc
	chmod 755 pathproc

install: all
	@echo read doc/Install

clean:
	rm -f *.o *.ln a.out core

clobber: clean
	rm -f smail rmail lcasep pathproc svbinmail
