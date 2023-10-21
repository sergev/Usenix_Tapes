# @(#)Makefile	1.1 (TRW) 1/14/86
CFLAGS = -O
OBJECTS = boolean.o free.o has.o read.o space.o write.o
DEST = /usr/local

.c.o:
	${CC} ${CFLAGS} -c $*.c
	-ld -r -x $*.o
	mv a.out $*.o

all: libprofile.a

libprofile.a: ${OBJECTS}
	ar cr libprofile.a ${OBJECTS}
	ranlib libprofile.a
	chmod 644 libprofile.a

install: all
	mv libprofile.a ${DEST}/lib
	ranlib ${DEST}/lib/libprofile.a
	install -c profile.h /usr/include/local

clean:
	rm -f *.o
