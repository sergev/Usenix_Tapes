INCLIB = /usr/include/CC
CLASSLIB = ..
AS = /bin/as
CC = CC
CFLAGS = -I${CLASSLIB}
LIB = ../liboops
include make.all

.SUFFIXES: .s
.c.a:
	${CC} ${CFLAGS} -c $<
	ar rv $@ $*.o
	rm $*.o
.s.a:
	${AS} $< -o $*.o
	ar rv $@ $*.o
	rm $*.o
.c.s:
	${CC} ${CFLAGS} -Fc -..c $<
	cc -S -O $*..c

all: ${OBJECTS}

depend:
	echo 'OBJECTS =' *.s | sed -e 's#\([a-zA-Z0-9_]*\)\.[cs]#${LIB}.a(\1.o)#g' >make.all
	makedep -I'${CLASSLIB} ${INCLIB}' -a'${INCLIB}' *.c *.s *.h ${CLASSLIB}/*.h ${INCLIB}/*.h | sed -e 's#\([a-zA-Z0-9_]*\.o\)#${LIB}.a(\1)#g' >make.dep

include make.dep
