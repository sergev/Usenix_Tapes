CC = CC
I = /usr/include/CC
CCDEBUG =
OOPSLIBDIR = ../src
OOPSINCDIR = ../src
OOPSLIB = ${OOPSLIBDIR}/liboops.a
CFLAGS = -I${OOPSINCDIR} ${CCDEBUG}
LFLAGS = ${OOPSLIB}

include progs.all
include classes.all

.SUFFIXES: ..s
.c:
	${CC} ${CFLAGS} $< -o $* ${LFLAGS}
.c.o:
	${CC} ${CFLAGS} -c $<
.c..s:
	${CC} ${CFLAGS} +i -S $<

all:${CLASSES} ${PROGS}

teldir1: teldir1.o ${OOPSLIB}
	${CC} ${CCDEBUG} $@.o -o $@ ${LFLAGS}

teldir2: teldir2.o ${OOPSLIB}
	${CC} ${CCDEBUG} $@.o -o $@ ${LFLAGS}

teldir3: teldir3.o TelDict.o ${OOPSLIB}
	${CC} ${CCDEBUG} $@.o TelDict.o -o $@ ${LFLAGS}

teldir4: teldir4.o Record.o ${OOPSLIB}
	${CC} ${CCDEBUG} $@.o Record.o -o $@ ${LFLAGS}

depend:
	echo 'PROGS =' [a-z]*[a-z0-9].c | sed -e 's#\([a-zA-Z0-9]*\)\.c#\1#g' >progs.all
	echo 'CLASSES =' [A-Z]*[a-z0-9].c | sed -e 's#\([a-zA-Z0-9]*\)\.c#\1.o#g' >classes.all
	makedep -I'${I} ${OOPSINCDIR}' -a'${I}' *.c *.h ${OOPSINCDIR}/*.h ${I}/*.h >make.dep

clean:
	-rm *.o ${PROGS} *..c *..s

include make.dep
