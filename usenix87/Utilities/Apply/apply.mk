#	apply.mk -- makefile for "apply" utility

#	last edit:	85/10/20	D A Gwyn

#	SCCS ID:	@(#)apply.mk	1.1

PRODUCT = apply
MKFILE	= ${PRODUCT}.mk
CFILES	= ${PRODUCT}.c
OBJS	= ${PRODUCT}.o
LDFLAGS = -n
BINDIR	= /vld/bin
MANDIR	= /usr/5lib/man/local/man1
BINPERM	= 755
MANPERM	= 664
TESTDIR = .
INS	= cp

.DEFAULT:
	$(GET) $(GFLAGS) -p s.$@ > ${TESTDIR}/$@
	touch $@

all:		${PRODUCT} ${PRODUCT}.1

${PRODUCT}:	${OBJS}
	$(CC) -o ${TESTDIR}/$@ ${LDFLAGS} ${OBJS}
	size ${TESTDIR}/$@
	touch $@

print:		${PRODUCT}.1 ${MKFILE} ${CFILES}
	( nroff -Tlp -man ${TESTDIR}/${PRODUCT}.1 ; \
	  pr ${MKFILE} ${CFILES} ${TESTDIR}/${PRODUCT}.1 ) | lpr

lint:		${CFILES}
	lint ${CFILES} > ${PRODUCT}.lint

compare:	all
	cmp ${BINDIR}/${PRODUCT} ${TESTDIR}/${PRODUCT}
	cmp ${MANDIR}/${PRODUCT}.1 ${TESTDIR}/${PRODUCT}.1

install:	all
	${INS} ${TESTDIR}/${PRODUCT} ${BINDIR}
	chmod ${BINPERM} ${BINDIR}/${PRODUCT}
	${INS} ${TESTDIR}/${PRODUCT}.1 ${MANDIR}
	chmod ${MANPERM} ${MANDIR}/${PRODUCT}.1

clean:
	-if vax; then rm -f ${CFILES}; fi
	-rm -f ${OBJS} ${PRODUCT}.lint

clobber:	clean
	-if vax; then rm -f ${TESTDIR}/${PRODUCT}.1; fi
	-rm -f ${TESTDIR}/${PRODUCT}
