v7run:	compat.o v7run.o v7start.o v7traps.o unixemts.o dosig.o dofloat.o
	ld -e start -N -o v7run compat.o v7run.o v7start.o v7traps.o\
	unixemts.o dosig.o dofloat.o /lib/crt0.o -lIS -lc

v6run:	compat.o v6run.o v6start.o v6traps.o unixemts.o dosig.o dofloat.o
	ld -e start -N -o v6run compat.o v6run.o v6start.o v6traps.o\
	unixemts.o dosig.o dofloat.o /lib/crt0.o -lIS -lc

v7trc:	compat.o v7run.o v7start.o v7ttraps.o unixemts.o dosig.o dofloat.o
	ld -e start -N -o v7trc compat.o v7run.o v7start.o v7ttraps.o\
	unixemts.o dosig.o dofloat.o /lib/crt0.o -lIS -lc

v6trc:	compat.o v6run.o v6start.o v6ttraps.o unixemts.o dosig.o dofloat.o
	ld -e start -N -o v6trc compat.o v6run.o v6start.o v6ttraps.o\
	unixemts.o dosig.o dofloat.o /lib/crt0.o -lIS -lc

rtrun:	compat.o rtrun.o rtstart.o rttraps.o rtemts.o dosig.o dofloat.o
	ld -e start -N -o rtrun compat.o rtrun.o rtstart.o rttraps.o\
	rtemts.o dosig.o dofloat.o /lib/crt0.o -lIS -lc

rttrc:	compat.o rtrun.o rtstart.o rtttraps.o rtemts.o dosig.o dofloat.o
	ld -e start -N -o rttrc compat.o rtrun.o rtstart.o rttraps.o\
	rtemts.o dosig.o dofloat.o /lib/crt0.o -lIS -lc

all:	v7run v7trc v6run v6trc

compat.o: memsiz.s compat.s
	cat memsiz.s compat.s |as -o compat.o

v7run.o: defs.h unixhdr.h runcompat.c
	cc -c -O -DV7UNIX -DUNIX runcompat.c
	mv runcompat.o v7run.o

v6run.o: defs.h unixhdr.h runcompat.c
	cc -c -O -DV6UNIX -DUNIX runcompat.c
	mv runcompat.o v6run.o

rtrun.o: defs.h rt11.h runcompat.c
	cc -c -O -DRT11 runcompat.c
	mv runcompat.o rtrun.o

v7start.o: defs.h unixstart.c
	cc -c -O -DV7UNIX unixstart.c
	mv unixstart.o v7start.o

v6start.o: defs.h unixstart.c
	cc -c -O -DV6UNIX unixstart.c
	mv unixstart.o v6start.o

v7traps.o: defs.h unix7sys.h unixtraps.c
	cc -c -O -DV7UNIX unixtraps.c
	mv unixtraps.o v7traps.o

v6traps.o: defs.h unix6sys.h unixtraps.c
	cc -c -O -DV6UNIX unixtraps.c
	mv unixtraps.o v6traps.o

v7ttraps.o: defs.h unix7sys.h unixtraps.c
	cc -c -O -DV7UNIX -DTRACE unixtraps.c
	mv unixtraps.o v7ttraps.o

v6ttraps.o: defs.h unix6sys.h unixtraps.c
	cc -c -O -DV6UNIX -DTRACE unixtraps.c
	mv unixtraps.o v6ttraps.o

unixemts.o: unixemts.c
	cc -c -O unixemts.c

dosig.o: dosig.c
	cc -c -O dosig.c

dofloat.o: defs.h dofloat.c
	cc -c -O dofloat.c
