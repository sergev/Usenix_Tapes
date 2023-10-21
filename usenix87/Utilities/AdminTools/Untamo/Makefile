# Makefile for /usr/src/new/untamo
#
# Author : Craig Norborg, Purdue University Computing Center
# To configure this for your system, be sure to check out untamo.h
# Also, if compiling on generic 4.2BSD system, be sure to make dependancies
# or remove all occurances of "rld" stuff from dependancies lines.  -doc

# C compiler flags for PUCC.
# Note, when the DEBUG flag is set, the RLD_FILE is changed to /tmp/rlds.
#INCLUDE = 
#DEFS = -DPUCC -DBSD4_2
#CFLAGS = -O ${DEFS} ${INCLUDE}
#LIBS = -lacct

# C compiler flags for generic BSD4_2
INCLUDE = 
DEFS = -DBSD4_2
CFLAGS = -O ${DEFS} ${INCLUDE}
LIBS =

#install flags
DEST = /usr/new/etc
FDEST = /usr/local/lib
OWNER = doc
GROUP = system
MODE = 751
SHMODE = 755
FMODE = 644

# Source files to be mkdepended
SRC = insque.c list.c untamo.c warn.c zap.c
SRCl = parse.y scan.l
SRCg = parse.c scan.c
HDR = ttydev.h untamo.h
OBJ = insque.o list.o parse.o scan.o untamo.o warn.o zap.o

# programs that need explicit make lines, plain files
NSTD = untamo
FILE = untamo.cf doc
MAN  = untamo.8l untamo.cf.5l

all: ${NSTD} ${FILE}

clean:
	rm -f a.out ${OBJ} core errs lint.errs Makefile.bak *.s tags \
		${SRCg} ${NSTD} y.tab.* lex.yy.c

depend: ${SRC} ${SRCg} ${HDR}
	maketd -a ${DEFS} ${INCLUDE} ${SRC} ${SRCg}

install: all 
	-for p in ${NSTD}; do \
		install -c -m ${MODE} -o ${OWNER} -g ${GROUP} -s $$p ${DEST}; \
	done
	for p in ${FILE}; do \
		install -c -m ${FMODE} -o ${OWNER} -g ${GROUP} -s $$p ${FDEST};\
	done

lint: ${SRC} ${SRCg}
	lint -hxn ${DEFS} ${SRC} ${SRCg}
	
print: 
	lpr -p -Pstaff -J"Untamo Source" Makefile ${SRCl} ${SRC} ${HDR}

shar:
	shar README Makefile ${FILE} ${MAN} ${SRC} ${SRCl} ${HDR} > untamo.shar

source: ${SRC} ${SRCl} ${HDR} ${MAN} ${FILE}

spotless:
	rm -f a.out ${OBJ} core errs lint.errs Makefile.bak y.tab.* yacc.act\
		yacc.tmp *.s ${NSTD} tags parse.c scan.c
	rcsclean ${SRC} ${SRCl} ${HDR} ${FILE} ${MAN}

tags: ${SRCS} ${SRCg} tags
	ctags ${SRC} ${SRCg}

${SRC} ${SRCl} ${HDR} ${FILE} ${MAN}:
	co $@

# rules for everybody in ${NSTD} go here
untamo: parse.o scan.o insque.o list.o untamo.o warn.o zap.o
	cc ${CFLAGS} -o untamo insque.o list.o parse.o scan.o untamo.o \
				warn.o zap.o ${LIBS}
y.tab.h: parse.c

parse.c:
	yacc -d parse.y
	mv y.tab.c parse.c

scan.c:
	lex scan.l
	mv lex.yy.c scan.c

# DO NOT DELETE THIS LINE - make depend DEPENDS ON IT
I=/usr/include
S=/usr/include/sys

insque.o: insque.c

list.o: $I/setjmp.h $I/stdio.h $S/file.h $S/types.h list.c untamo.h y.tab.h

untamo.o: $I/pwd.h $I/setjmp.h $I/signal.h $I/stdio.h $S/file.h $S/ioctl.h \
	$S/jioctl.h $S/stat.h $S/ttychars.h $S/ttydev.h $S/types.h $I/utmp.h \
	untamo.c untamo.h y.tab.h

untamo.o: $I/setjmp.h $I/stdio.h $S/types.h untamo.h

warn.o: $I/setjmp.h $I/signal.h $I/stdio.h $S/ioctl.h $S/jioctl.h \
	$S/ttychars.h $S/ttydev.h $S/types.h untamo.h warn.c y.tab.h

zap.o: $I/setjmp.h $I/stdio.h $S/file.h $S/ioctl.h $S/jioctl.h $S/ttychars.h \
	$S/ttydev.h $S/types.h $S/wait.h $I/utmp.h untamo.h zap.c

parse.o: $I/grp.h $I/setjmp.h $I/stdio.h $S/types.h parse.c untamo.h

scan.o: $I/stdio.h scan.c y.tab.h

# *** Do not add anything here - It will go away. ***
