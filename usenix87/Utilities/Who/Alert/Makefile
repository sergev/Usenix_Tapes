.SUFFIXES: .c .b .o .68
DAEMONOBJS=parse.o utils.o hunt.o time.o
DAEMONDESTDIR=/etc
DESTDIR=/usr/local

all: alert alertd

install: alert alertd
	rm -f ${DESTDIR}/alert
	cp alert ${DESTDIR}/alert
	install -m 755 -o root alertd ${DAEMONDESTDIR}/alertd

alertd: alertd.c  ${DAEMONOBJS} 
	cc -O -o alertd alertd.c ${DAEMONOBJS}

alert: alert.c gdate.o
	cc -O -o alert alert.c gdate.o

.c.o: $*.c
	cc -O -c $*.c

alertd.c: defs.h
alert.c : defs.h
