CC = CC
I = /usr/include/CC
CCDEBUG =
OOPSLIBDIR = /usr/lib
OOPSINCDIR = /usr/include/oops
OOPSLIB = ${OOPSLIBDIR}/liboops.a
CFLAGS = -I${OOPSINCDIR} ${CCDEBUG}
LFLAGS = ${OOPSLIB} ${CCDEBUG}

include progs.all
include classes.all

.SUFFIXES: .s
.c:
	${CC} ${CFLAGS} $< -o $* ${LFLAGS}
.c.o:
	${CC} ${CFLAGS} -c $<
.c.s:
	${CC} ${CFLAGS} +i -S $<

all:${CLASSES} ${PROGS}

form:form.o Form.o Field.o NumberField.o AlphaField.o
	${CC} form.o Form.o Field.o NumberField.o AlphaField.o -o form ${LFLAGS}

createdict:createdict.o DateDict.o
	${CC} createdict.o DateDict.o -o createdict ${LFLAGS}

datedict:datedict.o DateDict.o
	${CC} datedict.o DateDict.o -o datedict ${LFLAGS}

depend:
	echo 'PROGS =' [a-z]*[a-z0-9].c | sed -e 's#\([a-zA-Z0-9]*\)\.c#\1#g' >progs.all
	echo 'CLASSES =' [A-Z]*[a-z0-9].c | sed -e 's#\([a-zA-Z0-9]*\)\.c#\1.o#g' >classes.all
	makedep -I'${I} ${OOPSINCDIR}' -a'${I} ${OOPSINCDIR}' *.c *.h ${OOPSINCDIR}/*.h ${I}/*.h >make.dep

clean:
	-rm *.o ${PROGS} *..c *.s

include make.dep
