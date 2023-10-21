CC= cc
CFLAGS=-s -O
FILE= checkmail.c
OBJ= checkmail.o
LIBS=
BINDIR=.

tabkill:
	cc ${FILE} ${CFLAGS} ${LIBS} -o ${BINDIR}/checkmail
	-rm ${BINDIR}/newmail ${BINDIR}/M ${BINDIR}/NM
	ln ${BINDIR}/checkmail ${BINDIR}/newmail
	ln ${BINDIR}/checkmail ${BINDIR}/M
	ln ${BINDIR}/checkmail ${BINDIR}/NM
