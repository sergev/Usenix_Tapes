INCLIB = /usr/include/CC
CLASSLIB = ../src
VECLIB = ../src/vector
CC = CC
CFLAGS = -I${CLASSLIB} -I${VECLIB} -g
LFLAGS = -g ../src/vector/liboopsvec.a ../src/liboops.a -lm
include progs.all
include classes.all

.SUFFIXES: .d .s
.c:
	${CC} ${CFLAGS} $< -o $* ${LFLAGS}
.c.o:
	${CC} ${CFLAGS} -c $<
.c.d:
	${CC} ${CFLAGS} +i -g $< -o $*.d ${LFLAGS}
.c.s:
	${CC} ${CFLAGS} +i -S $<

all:${CLASSES} ${PROGS}

${PROGS}: $$@.o ${CLASSES} ../src/liboops.a ../src/vector/liboopsvec.a
	${CC} $@.o -o $@ ${CLASSES} ${LFLAGS}

depend:
	echo 'PROGS =' [a-z]*[a-z0-9].c | sed -e 's#\([a-zA-Z0-9]*\)\.c#\1#g' >progs.all
#	echo 'CLASSES =' [A-Z]*[a-z0-9].c | sed -e 's#\([a-zA-Z0-9]*\)\.c#\1.o#g' >classes.all
	makedep -I'${CLASSLIB} ${VECLIB} ${INCLIB}' -a'${INCLIB}' *.c *.h ${CLASSLIB}/*.h ${VECLIB}/*.h ${INCLIB}/*.h >make.dep

clean:
	-rm *.o ${PROGS} *..c *..s

include make.dep
