CC = CC
I = /usr/include/CC
CCDEBUG = -g
VECINCDIR = ../vector
VECLIB = ${VECINCDIR}/liboopsvec.a
OOPSINCDIR = ../src
OOPSLIB = ${OOPSINCDIR}/liboops.a
CFLAGS = -I${OOPSINCDIR} -I${VECINCDIR} ${CCDEBUG}
LFLAGS = ${CCDEBUG} ${VECLIB} ${OOPSLIB} -lm

include progs.all
include classes.all

.SUFFIXES: ..c .s
.c.o:
	${CC} ${CFLAGS} -c $<
.c..c:
	${CC} ${CFLAGS} -Fc -..c $<
.c.s:
	${CC} ${CFLAGS} +i -S -c $<

all: ${PROGS}

${PROGS}: $$@.o ${CLASSES} ${OOPSLIB} ${VECLIB}
	${CC} $@.o -o $@ ${CLASSES} ${LFLAGS}

depend:
	echo 'PROGS =' [a-z]*[a-z0-9].c | sed -e 's#\([a-zA-Z0-9]*\)\.c#\1#g' >progs.all
#	echo 'CLASSES =' [A-Z]*[a-z0-9].c | sed -e 's#\([a-zA-Z0-9]*\)\.c#\1.o#g' >classes.all
	makedep -I'${OOPSINCDIR} ${VECINCDIR} ${I}' -a'${I}' *.c *.h ${OOPSINCDIR}/*.h ${VECINCDIR}/*.h ${I}/*.h >make.dep

clean:
	-rm *.o ${PROGS} ${CLASSES} *..c *.s

include make.dep
