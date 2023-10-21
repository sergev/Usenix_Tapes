# $Header: Makefile,v 1.1 85/05/06 19:32:13 cspencer Exp $
CFLAGS= -O
INSTALL=/usr/local/bin/install
DESTDIR=/usr/local/bin

all: ofiles

ofiles:	 ofiles.o
	cc ofiles.o ${LIBES} -o ofiles

all.install: ofiles
	${INSTALL} ofiles ${DESTDIR} -m 755 -u root -g bin

rmobjects clobber clean:
	rm -f ofiles *.o *.out *.log core
