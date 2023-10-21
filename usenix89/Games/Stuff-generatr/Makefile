CFLAGS=	-O
CC=	/bin/cc

STD=	rnd 
DOC=	README

all:	${STD} ${DOC} 

${STD}: rnd.c
	${CC} ${CFLAGS} -o $@ $@.c

${DOC}: rnd.nro
	nroff -man rnd.nro >> README
