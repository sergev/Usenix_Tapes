#
# Makefile for mazed
#

CFILES =	curtime.c getsocket.c init.c join.c main.c send.c service.c \
		syserr.c ualarm.c verify.c work.c

OFILES =	curtime.o getsocket.o init.o join.o main.o send.o service.o \
		syserr.o ualarm.o verify.o work.o

HFILES =	../h/defs.h ../h/struct.h ../h/extern.h ../h/data.h


CFLAGS =	-O -DPLUS5

mazed:          $(OFILES) vers.o ../bem/libbem.a
		$(CC) $(CFLAGS) $(OFILES) vers.o ../bem/libbem.a -o mazed

lint:
		lint -z $(CFILES)

lint.serge:
		lint -haxb $(CFILES)

lint.port:
		lint -phaxcb $(CFILES)

ctags:
		ctags $(CFILES) $(HFILES)

clean:
		rm -f *.o core mazed *.out

init.o:          ../h/defs.h ../h/struct.h ../h/extern.h
join.o:          ../h/defs.h ../h/struct.h ../h/extern.h
main.o:          ../h/defs.h ../h/struct.h ../h/data.h
send.o:          ../h/defs.h ../h/struct.h ../h/extern.h
work.o:          ../h/defs.h ../h/struct.h ../h/extern.h
ualarm.o:        ../h/defs.h ../h/struct.h ../h/extern.h

vers.o:		${OFILES}
#		newvers vers.c			RMM
		${CC} ${CFLAGS} -c vers.c
