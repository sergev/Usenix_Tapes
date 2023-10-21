# $Header: Makefile,v 1.1 88/01/14 23:50:33 keith Exp $
CC = CC
I = /usr/include/CC
CCDEBUG =
LIB = liboops
CFLAGS = ${CCDEBUG}
LFLAGS = ${CCDEBUG} ${OOPSLIB}
# Target Directories
OOPSLIBDIR = /usr/lib
OOPSINCDIR = /usr/include/oops
OOPSSRCDIR = /usr/local/src/oops

include make.all

.SUFFIXES: ..c .s
.c.a:
	echo $< >>targetlist
.c..c:
	${CC} ${CFLAGS} +i -c $<
.c.s:
	${CC} ${CFLAGS} +i -S -c $<

.PRECIOUS: ${LIB}.a

all: always oopserrsx.h ${LIB}.a(regex.o) ${LIB}.a

always:
	-rm targetlist *.o
	make generic

oopserrsx.h: oopserrs.err
	csh -c 'if(-e /usr/bin/errcom) errcom oopserrs.err'
	csh -c 'if( ! -e /usr/bin/errcom) errgen oopserrs.err'
	sed -f oopserrs.sed <oopserrs.c >oopserrs.tmp
	mv oopserrs.tmp oopserrs.c
	sed -n -e "s/^\([A-Z][a-zA-Z]*\).*/int OOPS_\1	= OOPS__\1;/p" <oopserrs.err >oopserrsx.h

generic: Arraychar.h Arraychar.c

Arraychar.h: Array_h.m4
	echo "ARRAYDECLARE(Arraychar,char)" | m4 Array_h.m4 - >Arraychar.h

Arraychar.c: Array_c.m4 Arraychar.p
	m4 Array_c.m4 Arraychar.p >Arraychar.c

${LIB}.a(regex.o): regex/regex.c regex.h
#	cc -O -I. -D'SIGN_EXTEND_CHAR(c)=(((c) & 0x80) ? ((c) | 0xffffff80) : (c))' -c regex/regex.c
	cc -O -I. -c regex/regex.c
	touch targetlist

${LIB}.a: ${OBJECTS}
	${CC} ${CFLAGS} -c `cat targetlist`
	ar rv $@ *.o
	rm *.o
	-ranlib ${LIB}.a

install:
	cp ${LIB}.a ${OOPSLIBDIR}/${LIB}.a
	-ranlib ${OOPSLIBDIR}/${LIB}.a
	-mkdir ${OOPSINCDIR}
	-rm ${OOPSINCDIR}/*.h
	cp *.h ${OOPSINCDIR}
	-mkdir ${OOPSSRCDIR}
	-rm ${OOPSSRCDIR}/*.c ${OOPSSRCDIR}/Template_*
	cp *.c ${OOPSSRCDIR}
	cp Template_[hc] ${OOPSSRCDIR}

depend:
	-rm *..c
	make generic
	echo 'OBJECTS = ${LIB}.a(regex.o)' *.c | sed -e 's#\([a-zA-Z0-9_]*\)\.c#${LIB}.a(\1.o)#g' >make.all
	makedep -I'${I}' -a'${I}' *.c *.h ${I}/*.h | sed -e 's#\([a-zA-Z0-9_]*\.o\)#${LIB}.a(\1)#g' >make.dep

clean:
	-rm targetlist *.o *..c ${LIB}.a oopserrs.c oopserrsx.h

include make.dep
