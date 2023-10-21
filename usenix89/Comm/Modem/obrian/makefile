#	Makefile 1.0	3/20/86
#
# make file for Xmodem on Unix Protocol Package
#
#

CFLAGS=	-O -s
OBJS=	termio.o xmodemio.o
SOURCES= termio.c xmodemio.c xumodem.c
CXREF=	/usr/ucb/ctags -xw

xum:	makefile ${OBJS} xumodem.o
	${CC} ${CFLAGS} -o xum ${OBJS} xumodem.o -ltermlib

test:	${OBJS} test.o
	${CC} ${CFLAGS} -o txum ${OBJS} test.o

termio.o:	termio.c
	${CC} ${CFLAGS} -c termio.c

xmodemio.o:	xmodemio.c
	${CC} ${CFLAGS} -c xmodemio.c
	
${OBJS}:	termio.h

test.o:	test.c
	${CC} ${CFLAGS} -c test.c

clean:
	rm -f xum txum *.o

print:
	@pr makefile
	@pr termio.h ${SOURCES}
	@${CXREF} termio.h ${SOURCES} | pr -h XREF

lint:
	-lint -hbacvx ${SOURCES}
