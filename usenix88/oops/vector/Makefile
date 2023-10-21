CC = CC
I = /usr/include/CC
CCDEBUG =
OOPSINCDIR = ../src
LIB = liboopsvec
CFLAGS = -I${OOPSINCDIR} ${CCDEBUG}
include make.all

.SUFFIXES: ..c .s
.c.a:
	echo $< >>targetlist
.c.o:
	${CC} ${CFLAGS} -c $<
.c..c:
	${CC} ${CFLAGS} -Fc -..c $<
.c.s:
	${CC} ${CFLAGS} +i -S -c $<

.PRECIOUS: ${LIB}.a

all: always ${LIB}.a

${LIB}.a: ${OBJECTS}
	${CC} ${CFLAGS} -c `cat targetlist`
	ar rv $@ *.o
	rm *.o
	-ranlib ${LIB}.a

always:
	-rm targetlist *.o
	make generic

generic:
	make -f genericmake ${VECS}

depend:
	-rm *..c
	echo 'VECS =' *Vec.h | sed -e 's#\([a-zA-Z0-9_]*\)\.h#\1.c#g' >make.all
	make generic
	echo 'OBJECTS =' *.c | sed -e 's#\([a-zA-Z0-9_]*\)\.c#${LIB}.a(\1.o)#g' >>make.all
	makedep -I'${OOPSINCDIR} ${I}' -a'${I}' *.c *.h ${OOPSINCDIR}/*.h ${I}/*.h | sed -e 's#\([a-zA-Z0-9_]*\.o\)#${LIB}.a(\1)#g' >make.dep

clean:
	-rm targetlist *.o *Vec.c *..c *.s ${LIB}.a

include make.dep
