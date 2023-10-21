CFLAGS=-O

all:	uukill uuque

dir.o uukill.o uuque.o: dir.h

uukill: dir.o uukill.o
	cc $(CFLAGS) -o uukill uukill.o dir.o

uuque:	dir.o uuque.o
	cc $(CFLAGS) -o uuque uuque.o dir.o

