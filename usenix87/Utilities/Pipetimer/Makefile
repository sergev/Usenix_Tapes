CFLAGS   = 
LIBS     = timesPrint.o

all:	talker listener

talker: talker.c hdr.h timesPrint.o
	cc ${CFLAGS} $@.c -o $@ ${LIBS}

listener: listener.c hdr.h timesPrint.o
	cc ${CFLAGS} $@.c -o $@ ${LIBS}

timesPrint.o:	hdr.h

run:
	talker >>stats
