# MYINCL is where the include file "regexp.h" can be found, if it isn't
# in your /usr/include
MYINCL=$(HOME)/include

# Likewise for regexp.a, if the regexp routines aren't in your libc.a
MYLIBS=$(HOME)/lib

CFLAGS=-O -I$(MYINCL)

access : access.o regerror.o
	cc -O -o access access.o regerror.o $(MYLIBS)/regexp.a
