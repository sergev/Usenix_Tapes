# Makefile for slice
#
# Originally contributed at mailsplit, written by:
#   R E Quin, October 1986 University of Warwick (UK) Computer Science
#   warwick!req     +44 203 523193
#
# Modified and recontributed by:
#   Gary Puckering        3755 Riverside Dr.
#   Cognos Incorporated   Ottawa, Ontario
#   (613) 738-1440        CANADA  K1G 3N3
#
# This makefile is intended for the sys5 Augmented make.
# 
MAKE=make 
CLEAN=clean 
CC=cc 
HACKS= 
CFLAGS=-O $(HACKS)
# R is the root of the filesystem -- i.e. where to install things.
# The binaries are installed in $R/$(DESTDIR).
R=/usr/local
DESTDIR=$R/usr/bin 
MANDIR=$R/usr/man/manl
PROG=slice 

# PROG is what to make; DESTDIR is where to put it.
# HACKS are for -DBUGFIX style things.

# R is intended to be the root of the filesystem if it isn't "/"

# "make install " does a $(MAKE) $(CLEAN) at the end, so you can say
# CLEAN=  make -e install
# if you don't want to remove the garbage at the end, for example.
# This is useful primarily for testing the install: entry!

all: $(PROG)
 
slice: opts.h slice.o
	$(CC) -o $(PROG) slice.o
 
install: slice
	/bin/mv $(PROG) $(DESTDIR)
	/bin/cp $(PROG).1 $(MANDIR)/$(PROG).l
	$(MAKE) $(CLEAN)
 
clean: 
	rm -rf core *.o $(PROG) a.out
