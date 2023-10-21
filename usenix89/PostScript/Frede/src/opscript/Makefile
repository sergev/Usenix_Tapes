# SYS should be one of AUSAM, BSD, SYSV, V7
# This is only really important if VERBOSE (below) is 1.
SYS=SYSV
# VERBOSE should be 1 to generate postscript code to echo stuff
# back down the communication line. Otherwise 0.
VERBOSE=1
CFLAGS = -O -DALW -D$(SYS) -DVERBOSE=$(VERBOSE)
INSTALL = :
BINDIR=/bin
#I=/usr/include
I=/srce/include

all: lpscript ipscript


lpscript: $(BINDIR)/lpscript
$(BINDIR)/lpscript: lpscript.c $I/stdio.h $I/string.h pcom.c pscript.h
	$(CC) $(CFLAGS) -o lpscript lpscript.c pcom.c
	$(INSTALL) - lpscript bin 711 $(BINDIR)/lpscript

ipscript: $(BINDIR)/ipscript
$(BINDIR)/ipscript: ipscript.c $I/stdio.h pcom.c pscript.h
	$(CC) $(CFLAGS) -o ipscript ipscript.c pcom.c
	$(INSTALL) - ipscript bin 711 $(BINDIR)/ipscript

lint:	lint.ipscript lint.lpscript

lint.ipscript:
	lint -DALW -D$(SYS) ipscript.c pcom.c > ipscript.lint

lint.lpscript:
	lint -DALW -D$(SYS) lpscript.c pcom.c > lpscript.lint
