
# Makefile for ut

# David Stone
# Arete Systems, Inc 7/86

# System V.2 Release 2


# change BASEDIR if your uucp home directory is elsewhere

CFLAGS =	-O -DBASEDIR=\"/usr/spool/uucp/.Log\"

#  if the linker complains that tgetent, tgoto, tgetstr, etc undefined
#  change LIBS to also include -ltermcap or -ltermlib depending upon
#  which way it is called on your system.

LIBS =		-lcurses

OBJECTS =	ut.o

SRC =		ut.c Makefile

ut:		ut.c
		cc -o ut ut.c $(LIBS)
