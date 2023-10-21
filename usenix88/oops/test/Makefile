# $Header: Makefile,v 1.1 88/01/17 22:25:19 keith Exp $
#
# $Log:	Makefile,v $
# Revision 1.1  88/01/17  22:25:19  keith
# Initial revision
# 
CC = CC
I = /usr/include/CC
CCDEBUG = -g
OOPSINC = ../src
OOPSLIB = ../src/liboops.a
CFLAGS = -I${OOPSINC} ${CCDEBUG}
LFLAGS = ${OOPSLIB} ${CCDEBUG}

include progs.all
#include classes.all
include objects.all

.SUFFIXES: ..c .s
.c:
	${CC} ${CFLAGS} $< -o $* ${LFLAGS}
.c..c:
	${CC} ${CFLAGS} -Fc -..c $<
.c.o:
	${CC} ${CFLAGS} -c $<
.c.s:
	${CC} ${CFLAGS} +i -S $<

all:${PROGS}

${PROGS}: $$@.o ${OOPSLIB}
	${CC} $@.o -o $@ ${LFLAGS}

depend:
	-rm *..c
	echo 'PROGS =' [a-z]*[a-z0-9].c | sed -e 's#\([a-zA-Z0-9]*\)\.c#\1#g' >progs.all
#	echo 'CLASSES =' [A-Z]*[a-z0-9].c | sed -e 's#\([a-zA-Z0-9]*\)\.c#\1.o#g' >classes.all
	echo 'OBJECTS =' *.c | sed -e 's#\([a-zA-Z0-9]*\)\.c#\1.o#g' >objects.all
	makedep -I'${OOPSINC} ${I}' -a'${I}' *.c ${OOPSINC}/*.h ${I}/*.h >make.dep

clean:
	rm *.o ${PROGS} *..c *.s

include make.dep
