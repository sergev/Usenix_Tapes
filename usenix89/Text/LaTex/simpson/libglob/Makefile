# @(#)Makefile	1.1 (TRW) 1/14/86

CFLAGS = -O
OBJECTS = glob.o
DESTDIR = /
INCLUDE = /usr/include/local

.c.o:
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

all: libglob.a

libglob.a: ${OBJECTS}
	ar cr libglob.a ${OBJECTS}
	ranlib libglob.a
	chmod 644 libglob.a

install: all
	mv libglob.a ${DESTDIR}/usr/local/lib
	ranlib ${DESTDIR}/usr/local/lib/libglob.a
	install -c glob.h ${INCLUDE}/glob.h

clean:
	rm -f *.o
