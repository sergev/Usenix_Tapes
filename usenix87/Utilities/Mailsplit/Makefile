# Makefile for mailsplit
# R E Quin, October 1986 University of Warwick (UK) Computer Science
# warwick!req     +44 203 523193
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
MANDIR=$R/usr/man/man1
PROG=mailsplit 

# PROG is what to make; DESTDIR is where to put it.
# HACKS are for -DBUGFIX style things.

# R is intended to be the root of the filesystem if it isn't "/"

# "make install " does a $(MAKE) $(CLEAN) at the end, so you can say
# CLEAN=  make -e install
# if you don't want to remove the garbage at the end, for example.
# This is useful primarily for testing the install: entry!

all: $(PROG)
 
mailsplit: opts.h mailsplit.o
	$(CC) -o $(PROG) mailsplit.o
 
install: mailsplit
	/bin/mv $(PROG) $(DESTDIR)
	/bin/cp mailsplit.1 $(MANDIR)
	$(MAKE) $(CLEAN)
 
clean: 
	rm -rf core *.o $(PROG) a.out
