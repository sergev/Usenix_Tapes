#
# $Source: /local/projects/X/fed/RCS/Makefile,v $
# $Header: Makefile,v 1.16 87/04/07 19:17:32 jim Exp $
#
#                     Copyright (c) 1987 Cognition Inc.
#
# Permission to use, copy, modify and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies, and that both
# that copyright notice and this permission notice appear in supporting
# documentation, and that the name of Cognition Inc. not be used in
# advertising or publicity pertaining to distribution of the software
# without specific, written prior permission.  Cognition Inc. makes no
# representations about the suitability of this software for any purpose.
# It is provided "as-is" without express or implied warranty.
#
#							  Jim Fulton
#							  Cognition Inc.
#                                                         900 Tech Park Drive
# uucp:  ...!{mit-eddie,talcott,necntc}!ci-dandelion!jim  Billerica, MA
# arpa:  jim@athena.mit.edu, fulton@eddie.mit.edu         (617) 667-4800
#

#
# Font EDitor makefile, X10 fonts
#

#
# Building fed:  if you put it into the same tree as your other X sources
# you should be able to just say "make all".  If you don't have sources
# then you will probably want to change LOCALINCROOT to point to your include
# area (/usr/include) and add -Iinclude to INCLUDES so that it picks up the
# ones I've supplied.  I built this on a "standard" X installation with:
#
#		make LOCALINCROOT=include 
#

DESTDIR=

KEEP = Makefile .c .h .csh
CONFDIR = /usr/new
TOOLLIBS = 
LOCALINCROOT = ../include
CDEBUG = -O
INCLUDES = -I${LOCALINCROOT}
INLINE = ../inline/inline
C2 = /lib/c2
CFLAGS = ${CDEBUG} ${INCLUDES} 

LIB = libfont.a
LIBOBJS = getfont.o putfont.o Xemulation.o fontutils.o bitmap.o err.o \
	global.o bitmap.o 
FEDOBJS = fed.o 
BAOBJS = ba.o 
SCALEFONTOBJS = scalefont.o 
DISPLAYFILEOBJS = displayfile.o fontdisplay.o mapfile.o 
PROGS = fed ba scalefont displayfile # ic

#used by deps
FONTH = ${LOCALINCROOT}/X/vsinput.h ${LOCALINCROOT}/X/Xdev.h \
		${LOCALINCROOT}/X/vssite.h font.h vs100.h
#FONTH = ${LOCALINCROOT}/X/vsinput.h ${LOCALINCROOT}/X/Xdev.h \
#		${LOCALINCROOT}/X/X.h ${LOCALINCROOT}/X/vssite.h \
#		font.h vs100.h

#
# the following is for people who can use inline, otherwise just comment it out
#
# optimizer version
#
#.c.o:
#	${CC} ${CFLAGS} -DINLINE -c -S $*.c
#	${C2} $*.s | ${INLINE} | ${AS} -o $*.o
#	rm -f $*.s
#	-ld -x -r $*.o
#	chmod a-x a.out
#	mv a.out $*.o
#
# debug version
#
#.c.o:
#	${CC} ${CFLAGS} -DINLINE -c -S $*.c
#	${INLINE} $*.s | ${AS} -o $*.o
#	rm -f $*.s
#	-ld -r $*.o
#	chmod a-x a.out
#	mv a.out $*.o

all:  ${PROGS}

includes:

install:  ${PROGS}
	install -c -s -m 755 fed ${DESTDIR}/${CONFDIR}
	install -c -s -m 755 ba ${DESTDIR}/${CONFDIR}
	install -c -s -m 755 scalefont ${DESTDIR}/${CONFDIR}
	install -c -s -m 755 displayfile ${DESTDIR}/${CONFDIR}
#	install -c -m 755 ic ${DESTDIR}/${CONFDIR}
	@for i in ${PROGS}; do ls -lg ${DESTDIR}/${CONFDIR}/$$i; done

clean:
	rm -f a.out core ${PROGS} *.o *.a *~ \#* 

${LIB}:  ${LIBOBJS}
	rm -f $@
	ar cq $@ ${LIBOBJS}
	ranlib $@

fed:  ${FEDOBJS} ${LIB} 
	${CC} ${CFLAGS} -o $@ ${FEDOBJS} ${LIB} 

ba:  ${BAOBJS} ${LIB} 
	${CC} ${CFLAGS} -o $@ ${BAOBJS} ${LIB} 

scalefont:  ${SCALEFONTOBJS} ${LIB} 
	${CC} ${CFLAGS} -o $@ ${SCALEFONTOBJS} ${LIB} 

displayfile:  ${DISPLAYFILEOBJS} ${LIB} 
	${CC} ${CFLAGS} -o $@ ${DISPLAYFILEOBJS} ${LIB} -lX 

ic:  ic.csh
	cp $? $@; chmod a+x $@

${TOOLLIBS}/${LIB}:  ${LIB}
	(rm -f $@; tar cf - ${LIB} | (chdir ${TOOLLIBS}; tar xpBvf -))


# dependencies

#displayfile.o:  ${LOCALINCROOT}/X/Xlib.h
fed.o:  ${FONTH}
fed.o:  elem.h
#fontdisplay.o:  ${LOCALINCROOT}/X/Xlib.h
fontdisplay.o:  ${FONTH}
fontdisplay.o:  elem.h
fontdisplay.o:  mapfile.h
fontutils.o:  ${FONTH}
fontutils.o:  elem.h
getfont.o:  ${FONTH}
mapfile.o:  mapfile.h
putfont.o:  ${FONTH}
putfont.o:  elem.h
scalefont.o:  ${FONTH}

