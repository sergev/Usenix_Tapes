# @(#)Makefile	1.1 (TRW) 1/14/86
CFLAGS=	-O
DEST= /usr/local/lib
INCLUDE=/usr/include/local

.c.o:
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

all: libtrw.a

libtrw.a: shift.o getopt.o getunent.o
	ar cr libtrw.a shift.o getopt.o getunent.o
	ranlib libtrw.a

clean:
	rm -f *.o

install: all
	mv libtrw.a $(DEST)
	ranlib $(DEST)/libtrw.a
	cp universe.h $(INCLUDE)
