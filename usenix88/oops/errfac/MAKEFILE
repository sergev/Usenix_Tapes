CC = CC
CCDEBUG =
I = /usr/include/CC
LIB_ID = C
CFLAGS = ${CCDEBUG}
LFLAGS = ${CCDEBUG}
# Target Directories
CLIBDIR = /usr/lib
ERRDIR = /usr/local/bin

OBJECTS = ErrFac.o dofile.o errors.o gettok.o

.SUFFIXES: .s .i

.c.o:
	${CC} ${CFLAGS} -c $<
.c.s:
	${CC} ${CFLAGS} +i -S -c $<

all: prep ${OBJECTS} errgen testerrs.h errtst verify

errgen:  ErrFac.o gettok.o dofile.o errgen.o
	${CC} ErrFac.o gettok.o dofile.o errgen.o -o $@ ${LFLAGS}
#	strip errgen

errtst:  errtst.o errors.o testerrs.o
	${CC} errtst.o errors.o testerrs.o -o $@ ${LFLAGS}

testerrs.h: errgen testerrs.err
	errgen testerrs
	${CC} -I. ${CFLAGS} -c testerrs.c

verify: errtst
	csh -c 'if ( ! { (errtst |& cmp -s - errtst.v) } ) exit 1'

prep:
	-csh -c 'if ( ! -f /etc/errgen_tab ) cp errgen_tab /etc'

install:
	-mkdir ${ERRDIR}
	cp errgen ${ERRDIR}
	-mkdir ${I}
	cp errlib.h ${I}
	cp errors.h ${I}
	ar rv ${CLIBDIR}/lib${LIB_ID}.a errors.o
	-ranlib ${CLIBDIR}/lib${LIB_ID}.a
	
clean:
	-rm *.o errgen errtst testerrs.h testerrs.c

ErrFac.o: ErrFac.h ${I}/string.h
ErrFac.o: ${I}/stdio.h ${I}/stream.h 
dofile.o: dofile.h ${I}/string.h
dofile.o: ${I}/fcntl.h ${I}/osfcn.h
dofile.o: ${I}/stream.h ${I}/stdio.h
errgen.o: ${I}/stream.h ${I}/string.h ${I}/osfcn.h
errgen.o: ${I}/fcntl.h ErrFac.h gettok.h
errgen.o: dofile.h ${I}/stdio.h
errors.o: errors.h ${I}/stdio.h
errors.o: errlib.h ${I}/stream.h
gettok.o: gettok.h
testerrs.o: testerrs.h
errtst.o: errlib.h testerrs.h