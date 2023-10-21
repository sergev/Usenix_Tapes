#
# $Header: Makefile,v 1.6 86/07/19 16:53:52 gworek Exp $
#
# Makefile for staticon

SRCS =	staticon.c
OBJS =	staticon.o
DEST =	staticon
INSTALLDIR = /usr/local/bin

ICONS = batsignal.icon gort.icon marvin.icon
ICONDIR = /usr/include/local

LFLAGS = -lsuntool -lsunwindow -lpixrect
CFLAGS = -O -s

${DEST}: ${OBJS}
	cc ${CFLAGS} -o ${DEST} ${OBJS} ${LFLAGS}

install:
	install ${DEST} ${INSTALLDIR}
	cp ${ICONS} ${ICONDIR}

clean:
	rm -f *.o ${DEST}

lint: ${HDRS} ${SRCS}
	lint ${DFLAGS} ${SRCS}
