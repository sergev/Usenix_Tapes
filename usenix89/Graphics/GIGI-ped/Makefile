#	Makefile for ped, version 1.
#
#	Installation parameters:
BINDIR=$$HOME/bin
MANDIR=/dev/null
#
CFLAGS= -O
SOURCES=Makefile gigi.h ped.c ped.1
all: ped
ped: ped.o gigi.h
	cc -o ped ped.o
install: ped ped.1
	install -c ped $(BINDIR)
	install -c ped.1 $(MANDIR)
shar: ped.shar
ped.shar: $(SOURCES)
	shar $(SOURCES) > ped.shar
clean: rm -f *.o ped
