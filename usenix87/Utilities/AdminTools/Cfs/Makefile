#
#  Makefile for cfs
#    Rex Sanders, USGS, 12-86
#
CFLAGS=-O
OBJ=	addname.o cavn.o cfs.o create.o deco.o delete.o mkmode.o \
	movefile.o openread.o openwrite.o poparg.o print2stat.o \
	printstat.o readstat.o statcheck.o statcomp.o table.o update.o \
	writestat.o
PRINTER=enscript -q

cfs:  ${OBJ}
	cc ${CFLAGS} -o cfs ${OBJ}


${OBJ}: cfs.h

lint:
	lint -bchx *.h *.c

print:
	${PRINTER} Makefile *.h *.c

test:  cfs
	echo `pwd`/*.c `pwd`/test.statfile | ./cfs -cv test.statfile
	-./cfs -sv	test.statfile
	./cfs -t	test.statfile
	./cfs -tv	test.statfile	`pwd`/test.statfile
	./cfs -dv	test.statfile	`pwd`/test.statfile
	-./cfs -sv	test.statfile
	./cfs -tv	test.statfile	`pwd`/test.statfile
	./cfs -uv	test.statfile	`pwd`/test.statfile
	-./cfs -sv	test.statfile	`pwd`/test.statfile
	./cfs -uv	test.statfile	`pwd`/test.statfile
	./cfs -t	test.statfile	`pwd`/test.statfile
	-./cfs
	-./cfs -junkt
	-./cfs -t
	-./cfs -t	NONEXISTENTFILE

install: cfs
	install -s -o bin -g bin -m 0111 cfs /usr/local/bin

clean:
	/bin/rm -f ${OBJ} cfs cfs.?????? test.statfile make.out lint.out
