# :ts=8
# Makefile for robotroff.  I'm not an expert at makefiles,
# so please forgive my stupidity.

OBJECTS =	_main.o robotroff.o functions.o sprites.o rnd.o

robotroff: $(OBJECTS)
	ln $(OBJECTS) +cd -lc -o robotroff

robotroff.o functions.o: robotroff.h
