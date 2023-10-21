# Makefile for bitstring macros
# vix 26feb87 [written]
# vix 25mar87 [added test program]
#

# INCDIR might be /usr/local/include or /usr/include/local or some
# variant, but your cc(1) may not be looking there.  If you put it
# somewhere your cc(1) doesn't usually look for include files, you
# will have to compile things with -I or this one won't be found.

INCDIR=/usr/include

# MANEXT and MANDIR will vary from system to system, but usually on
# a BSD you put the man page for foobar in /usr/man/manl/foobar.l.
# On SysV, start looking in /usr/catman, and after that, you're on
# your own.

MANEXT=l
MANDIR=/usr/man/man$(MANEXT)

all		:	bitstring.3 bitstring.h
			cp bitstring.3 $(MANDIR)/bitstring.$(MANEXT)
			cp bitstring.h $(INCDIR)

test		:	test.c
			cc -O -o test test.c
