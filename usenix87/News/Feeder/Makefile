CC=CC
CFLAGS=-O
CONFIGOBJS=main.o log.o misc.o
OBJS=${CONFIGOBJS} feed.o batch.o io.o
INSTALLDIR=/usr/lib/news/

newsfeed:	${OBJS}
		${CC} -o $@ ${OBJS}

${OBJS}:	defs.h

${CONFIGOBJS}:	config.h

install:	newsfeed newsfeedlist.sh newsfeed.man newsfeedlist.man
		install -o news -g news -s newsfeed ${INSTALLDIR}
		cp newsfeedlist.sh ${INSTALLDIR}newsfeedlist
		chmod 755 ${INSTALLDIR}newsfeedlist
		chown news ${INSTALLDIR}newsfeedlist
		chgrp news ${INSTALLDIR}newsfeedlist
		cp newsfeed.man /usr/man/mann/newsfeed.n
		cp newsfeedlist.man /usr/man/mann/newsfeedlist.n

clean:
		rm -f newsfeed *.o *..o *..c core
