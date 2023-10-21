#
# makefile for intermachine search
#
# if you don't have AF_INET sockets available on your machine
# (i.e. no network stuff) then take out the flag -DINET
#
# run "make install" as ${ROOT}
#
OWNER=	play
OWNERID= 27
#ROOT=	root
ROOT=	${OWNER}
GROUP=	staff
OBJDIR= /usr/local/src/games/search
LIBDIR=	/usr/games/lib/search

OPTIONS=-DLIBDIR=\"${LIBDIR}
CFLAGS=	${OPTIONS} -DDEBUG -O -DOWNERID=$(OWNERID)

HEADS=	defines.h structs.h
CFILES=	aliens.c bursts.c cmds1.c cmds2.c groups.c init.c\
	io.c lists.c main.c planet.c playmap.c scab.c tgoto.c\
	util.c score.c signal.c ipc.c
OBJECTS=aliens.o bursts.o cmds1.o cmds2.o groups.o init.o\
	io.o lists.o main.o planet.o playmap.o scab.o tgoto.o\
	util.o score.o signal.o ipc.o

all:	search sscore searchd

search:	search.o excl.o
	cc ${CFLAGS} search.o excl.o -lcurses -o search

search.o:	$(HEADS)

searchwho: searchwho.c ${HEADS}
	cc ${CFLAGS} searchwho.c -o searchwho

sscore:	sscore.c ${HEADS}
	cc ${CFLAGS} sscore.c -o sscore

searchd:	${OBJECTS}
	cc ${CFLAGS} ${OBJECTS} -o searchd

${OBJECTS}: ${HEADS}

install: searchd search sscore searchwho
	-mkdir ${LIBDIR}
	chmod 775 ${LIBDIR}
	install -s -o ${ROOT} -g ${GROUP} -m 6751 searchd ${LIBDIR}/searchd
	install -s -o ${OWNER} -g ${GROUP} -m 751 sscore ${OBJDIR}/sscore
	install -s -o ${OWNER} -g ${GROUP} -m 751 search ${OBJDIR}/search
	touch ${LIBDIR}/points
	touch ${LIBDIR}/errlog
	chown ${OWNER} ${LIBDIR} ${LIBDIR}/points ${LIBDIR}/errlog
	chgrp ${GROUP} ${LIBDIR} ${LIBDIR}/points ${LIBDIR}/errlog
	chmod 664 ${LIBDIR}/points ${LIBDIR}/errlog

lint:
	lint -hxa ${OPTIONS} aliens.c bursts.c cmds1.c cmds2.c \
	groups.c init.c io.c lists.c main.c planet.c playmap.c \
	scab.c tgoto.c util.c score.c signal.c ipc.c
	

profile:
	cc -pg -o searchd.pro ${OBJECTS}

clean:
	rm -f *.o searchd sscore search searchwho core a.out ERRS errs

tags:	${CFILES}
	ctags ${CFILES}

distrib:	${CFILES} ${HEADS} Rules.ms README Makefile search.6
	shar	search.c sscore.c searchwho.c README	>search1.shar
	shar	Rules.ms Makefile search.6		>search2.shar
	shar	main.c ipc.c				>search3.shar
	shar	defines.h structs.h cmds1.c cmds2.c	>search4.shar
	shar	init.c util.c io.c tgoto.c score.c	>search5.shar
	shar	playmap.c scab.c signal.c groups.c planet.c \
		aliens.c bursts.c lists.c		>search6.shar
