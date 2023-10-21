# Makefile for Ease Translator (et).
#
#	$Header: /usr/src/local/etc/ease/RCS/Makefile,v 1.4 85/10/29 22:57:06 jss Exp $
#
#	James S. Schoner, Purdue University Computing Center,
#			  West Lafayette, Indiana  47907
#
#	Copyright (c) 1985 by James S. Schoner
#
#	All rights reserved.
#

INCLUDE =

DEST = /usr/local/etc

#OWNER = binary
OWNER = bin
#GROUP = system
GROUP = sys
MODE = 751

DEFS =
CFLAGS = -O ${DEFS} ${INCLUDE}

LP = lpr
LPFLAGS = -J"Ease Source"

HDR = symtab.h
SRC = main.c emitcf.c errors.c idman.c strops.c symtab.c
LST = Makefile lexan.l parser.y ${HDR} ${SRC}
DEP = y.tab.c lex.yy.c ${SRC}
OBJ = y.tab.o lex.yy.o main.o emitcf.o errors.o idman.o strops.o symtab.o

all: et

et: ${OBJ}
	cc ${CFLAGS} -o et ${OBJ} -ll

clean: FRC
	rm -f et *.o lex.yy.c y.tab.c y.output yacc.acts yacc.tmp \
	      lexdefs.h y.tab.h errs Makefile.bak

depend: ${DEP} ${HDR}
	maketd -a ${DEP}

install: et FRC
	install -c -m ${MODE} -o ${OWNER} -g ${GROUP} -s et ${DEST}

lint:   ${DEP} symtab.h FRC
	lint -hxn ${DEP}

print:  ${LST} FRC
	@pr -f ${LST} | ${LP} ${LPFLAGS}

spotless: clean FRC
	rcsclean ${LST}

lexdefs.h y.tab.c: parser.y
	yacc -d parser.y
	-(cmp -s y.tab.h lexdefs.h || cp y.tab.h lexdefs.h)

lex.yy.c: lexan.l
	lex lexan.l

${HDR} ${SRC} lexan.l parser.y:
	co $@

FRC:
	

# DO NOT DELETE THIS LINE - maketd DEPENDS ON IT
# Dependencies generated at: Thu Oct 17 14:55:17 EST 1985

emitcf.o: symtab.h
emitcf.o: /usr/include/stdio.h
emitcf.o: emitcf.c
errors.o: /usr/include/stdio.h
errors.o: errors.c
idman.o: symtab.h
idman.o: /usr/include/stdio.h
idman.o: idman.c
lex.yy.o: lexdefs.h
lex.yy.o: symtab.h
lex.yy.o: /usr/include/stdio.h
lex.yy.o: lex.yy.c
main.o: /usr/include/stdio.h
main.o: main.c
strops.o: symtab.h
strops.o: /usr/include/stdio.h
strops.o: /usr/include/strings.h
strops.o: strops.c
symtab.o: symtab.h
symtab.o: /usr/include/ctype.h
symtab.o: /usr/include/stdio.h
symtab.o: symtab.c
y.tab.o: symtab.h
y.tab.o: /usr/include/stdio.h
y.tab.o: y.tab.c

# DO NOT ADD ANYTHING HERE - it will go away.
