#	makefile	4.3	81/06/21
#
# makefile for intermachine communications package
#
# Files are:
#	/etc/remote		remote host description file
#	/etc/phones		phone number file, owned by ${OWNER} and
#				  mode 644
#	${ADM}/aculog		ACU accounting file, owned by ${OWNER} and
#				  mode 644 {if ACULOG defined}
# Presently supports:
#	BIZCOMP
#	DEC DF02-AC, DF03-AC
#	DEC DN-11/Able Quadracall {untested}
#
# Configuration defines:
#	BIZCOMP, DF02, DN11	ACU's supported
#	ACULOG			turn on tip logging of ACU use
#	PRISTINE		no phone #'s put in ACU log file
#	CONNECT			worthless command
#	DEFBR			default baud rate to make connection at
#	DEFFS			default frame size for FTP buffering of
#				writes on local side

ADM=/usr/adm
BIN=/usr/bin
OWNER=uucp
CFLAGS=-O -DVMUNIX -DDEFBR=300 -DDEFFS=BUFSIZ -DDN11 -DACULOG
LDFLAGS=-z
OBJECTS=acu.o acutab.o biz.o cmds.o cmdtab.o df.o dn11.o hunt.o lock.o log.o\
	remote.o tip.o tipout.o value.o vars.o

SOURCES=acu.c acutab.c biz.c cmds.c cmdtab.c df.c dn11.c hunt.c lock.c log.c\
	remcap.c remote.c tip.c tipout.c uucplock.c value.c vars.c
CXREF=/usr/ucb/ctags -xw

tip:	${OBJECTS} remcap.o uucplock.o
	${CC} -o tip ${LDFLAGS} ${OBJECTS} remcap.o uucplock.o

all:	tip aculog

install: all
	cp tip ${BIN}/tip
	chmod 4711 ${BIN}/tip
	chown ${OWNER} ${BIN}/tip
	cp aculog /etc/aculog
	cp /dev/null ${ADM}/aculog
	chown ${OWNER} ${ADM}/aculog
	chmod 600 ${ADM}/aculog
	@echo "create /etc/remote and /etc/phones"

aculog:	aculog.c
	${CC} -o aculog ${LDFLAGS} aculog.c
	chmod 4711 aculog
	chown ${OWNER} aculog

${OBJECTS}: tip.h

tags:	${SOURCES} tip.h
	ctags -w ${SOURCES} tip.h

clean:
	rm -f aculog tip *.o

print:
	@pr README TODO makefile
	@size *.o | pr -h SIZES
	@${CXREF} ${SOURCES} tip.h | pr -h XREF
	@ls -l | pr 
	@pr tip.h ${SOURCES}

lint:
	-lint -hbacvx ${CFLAGS} ${SOURCES}
