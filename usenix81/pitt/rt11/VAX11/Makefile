#
#  RT11 emulator makefile for 4.0BSD unix
#
CFLAGS= -O -DVAX -DVIRTUAL
OSUBRS= rtio.o csi.o junk.o printf.o tty.o

rtrun:	savebits crt0.o trap.o rtrun.o $(OSUBRS)
	ld -N -r -d -o rtrun crt0.o trap.o rtrun.o $(OSUBRS) -lc
	nm -u rtrun
	savebits rtrun
	chmod 755 rtrun
	strip rtrun

rtabs:	memalloc.o trap.o rtrun.o $(OSUBRS) /lib/crt0.o
	ld -N -o rtabs /lib/crt0.o memalloc.o trap.o rtrun.o $(OSUBRS) -lc
	strip rtabs

rttrc:	savebits crt0.o trap.o rttrc.o $(OSUBRS)
	ld -N -r -d -o rttrc crt0.o trap.o rttrc.o $(OSUBRS) -lc
	nm -u rttrc
	savebits rttrc
	chmod 755 rttrc
	strip rttrc

rttrc.o: rt.c
	cc -c $(CFLAGS) -DTRACE rt.c
	mv rt.o rttrc.o

rtrun.o: rt.c
	cc -c $(CFLAGS) rt.c
	mv rt.o rtrun.o

savebits: savebits.c /usr/include/a.out.h
	cc -O -o savebits savebits.c

clean:
	rm -f savebits *.o rtrun rttrc rtabs
