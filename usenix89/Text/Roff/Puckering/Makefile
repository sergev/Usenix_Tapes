# Makefile for roff
# Gary Puckering, Cognos Inc.   December 8, 1986
# 
# This makefile is intended for the sys5 Augmented make.
# 
MAKE=make 
CLEAN=clean
# R is the root of the filesystem -- i.e. where to install things.
# The binaries are installed in $R/$(DESTDIR).
R=/usr
DESTDIR=$R/local/bin
MANDIR=$R/man/manl
PROG=roff

# PROG is what to make; DESTDIR is where to put it.

# R is intended to be the root of the filesystem if it isn't "/"

# "make install " does a $(MAKE) $(CLEAN) at the end, so you can say
# CLEAN=  make -e install
# if you don't want to remove the garbage at the end, for example.
# This is useful primarily for testing the install: entry!

install: 
	/bin/chmod a+r,u+w $(PROG) $(PROG).l
	/bin/chmod a+x $(PROG)
	/bin/cp $(PROG) $(DESTDIR)
	/bin/cp $(PROG).l $(MANDIR)
	$(MAKE) $(CLEAN)
 
clean: 
	sccs clean
