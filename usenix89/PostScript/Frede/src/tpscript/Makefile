# INSTALL=cp
INSTALL=:
I=..
ROOT=
INSDIR=/usr/bin
#	-DUQMINMET for hashed character name lookup
SYS=SYSV
VERBOSE=1	# 1 to turn on stuff; 0 to turn off
LOCALSYS=	# for various local mods
CFLAGS= -O
LDFLAGS= -n
DFLAGS= -DUQMINMET -D$(SYS) -DALW -DVERBOSE=$(VERBOSE) -D$(LOCALSYS)
CFILES=tpscript.c pcom.c stringdefs.c spline.c sfont2.c hash.c
OFILES=tpscript.o pcom.o stringdefs.o spline.o sfont2.o hash.o

.c.o:
	$(CC) $(CFLAGS) $(DFLAGS) -c $<

all tpscript:	$(OFILES)
	$(CC) $(LDFLAGS) -o tpscript $(OFILES)

install:	tpscript
	$(INSTALL) tpscript $(ROOT)$(INSDIR)/dalw
	chown bin $(ROOT)$(INSDIR)/dalw
	chgrp bin $(ROOT)$(INSDIR)/dalw
	chmod 711 $(ROOT)$(INSDIR)/dalw

lint:	tpscript.lint

tpscript.lint:	 $(CFILES)
	lint -D$(SYS) $(CFILES) > tpscript.lint

clean:
	rm -f *.o tpscript.lint

clobber:	clean
	rm tpscript stringdefs.h sfont2defs.H

stringdefs.h:	stringdefs.H
	sed -f gendefs.sed stringdefs.H >stringdefs.h

sfont2defs.h:	sfont2defs.H
	sed -f gendefs.sed sfont2defs.H >sfont2defs.h

hash.o:		hash.h tpscript.h
pcom.o:		pscript.h
sfont2.o:	sfont2defs.h tpscript.h
spline.o:	tpscript.h
stringdefs.o:	hash.h stringdefs.h
tpscript.o:	tpscript.h dev.h pscript.h
