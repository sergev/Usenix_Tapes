HOME = /usr/campbell
BIN = $(HOME)/bin
GROUP = sys
OWNER = bin

#	set DFLAGS equal to:
#	   -DVENIX	if using VENIX
#	   -DSYSV	if using SYSTEM V
#	   -DVOID	if compiler doesn't understand 'void'
#	   -DMINIEXCH	if using the DEC mini-exchange
#	   -DXDIR=\"fullpath_name/x10\" if not using default of "."
DFLAGS = -DVENIX -DXDIR=\"$(HOME)/X10\" -DMINIEXCH

CFLAGS = -O $(DFLAGS)
LDFLAGS = -z -i
#LIBS = -lc_s		# uncomment if using shared libraries

SRCS =	data.c date.c delete.c diagnostic.c dump.c fdump.c \
	finfo.c fload.c info.c getslot.c message.c miniexch.c \
	monitor.c prints.c readid.c reset.c schedule.c setclock.c \
	tty.c unit.c x10.c xread.c

OBJS =	data.o date.o delete.o diagnostic.o dump.o fdump.o \
	finfo.o fload.o info.o getslot.o message.o miniexch.o \
	monitor.o prints.o readid.o reset.o schedule.o setclock.o \
	tty.o unit.o x10.o xread.o

x10:	$(OBJS)
	cc $(LDFLAGS) -o x10 $(OBJS) $(LIBS)
	if [ -f /usr/bin/mcs ]; then mcs -c x10; fi
	chgrp $(GROUP) x10
	chmod 2755 x10
	chown $(OWNER) x10

$(OBJS): x10.h

install: x10
	mv x10 $(BIN)

lint:
	lint $(DFLAGS) $(SRCS)

shar:	x10.shar.1 x10.shar.2

x10.shar.1:
	shar README REVIEW Makefile x10.[1h] >x10.shar.1

x10.shar.2:
	shar $(SRCS) > x10.shar.2

clean:
	rm -f *.o

clobber: clean
	rm -f x10
