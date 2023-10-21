#
#  help Makefile, 1.1	7/27/84
#
CFLAGS= -O
SRCS= help.h help.c
DESTDIR=

help:	help.o 
	${CC} ${CFLAGS} help.o -o help

install: help
	install -s -c help $(DESTDIR)/usr/local/bin

help.o: help.h /usr/include/sys/file.h /usr/include/sys/param.h

ci:
	ci -l ${SRCS}

clean:  
	rm -f *.o help

