FILES = rcheck.o net.o
CFLAGS = -O -DBSD
# If your system doesn't use 4.2BSD remove the two previous lines
# and remove the "#" at the beginning of the following two lines.
# FILES = rcheck.o
# CFLAGS = -O -UBSD

NAME = rcheck
DESTDIR = /usr/local

rcheck : ${FILES} rcheck.h
	cc ${CFLAGS} -o ${NAME} ${FILES}

rcheck.h : ${FILES}

install :
	mv ${NAME} ${DESTDIR}
	mv rcheck.1 /usr/man/manl
