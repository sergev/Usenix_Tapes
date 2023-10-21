AS = as
LIB = ../src/liboops.a

.SUFFIXES: .s
.s.o:
	${AS} $< -o $*.o

all: exchj.o
	ar rv ${LIB} $?
	-ranlib ${LIB}

install:

clean:
	-rm *.o *..c
