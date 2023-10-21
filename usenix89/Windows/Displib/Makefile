#
# MAKEFILE FOR THE DISPLAY LIBRARY "DISPLIB"
#
# Donated into the public domain for any use whatever by anyone.
# Please retain this header.
#
# This display library is a seriously modified version of a
# display library posted to USENET several years ago. Many thanks.
#
# * **************************************************************** *
# * Gary K. Sloane/Computer Sciences Corporation/San Diego, CA 92110 *
# * DDD: (619) 225-8401                                              *
# * MILNET: sloane@nosc.ARPA                                         *
# * UUCP: [ihnp4,akgua,decvax,dcdwest,ucbvax]!sdcsvax!noscvax!sloane *
# * **************************************************************** *
#

# where should the archive be installed?
DESTDIR = .

# where is the displib.h include file 
IDIR = .

# flags to cc and ld
CFLAGS =	-O

all: libdisp.a test

libdisp.a: libdisp.o
	ar cru libdisp.a libdisp.o
	ranlib libdisp.a

test: test.o libdisp.a
	cc -o test test.o libdisp.a -ltermlib

.c.o:
	${CC} -I${IDIR} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

