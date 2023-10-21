SRCS =	keepnews.c
OBJS =	keepnews.o
BINDIR= /usr/local
CFLAGS= -O

keepnews:	${OBJS}
	${CC} -o keepnews ${OBJS}

install: keepnews
	install -m 6755 -s keepnews ${BINDIR}/keepnews
	chown news ${BINDIR}/keepnews
	chgrp 0 ${BINDIR}/keepnews

clean:
	rm -f *.o

print:	${SRCS}
	lpr -p ${SRCS} Makefile
