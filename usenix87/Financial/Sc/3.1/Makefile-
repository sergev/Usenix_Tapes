
# Specify the name of the program.
# All documentation and installation keys on this value.
# 
name=sc

EXDIR=/a/rgb/bin
MANDIR=/usr/man/man1
SRC1=sc.h sc.c lex.c gram.y interp.c
SRC2=cmds.c eres.sed sres.sed makefile cvt.sed
DOCS=README $(name).man $(name).doc

#Set SIMPLE for lex.c if you don't want arrow keys or lex.c blows up
#SIMPLE=-DSIMPLE

#Use this for system V.2
#CFLAGS= -O -DSYSV
#LIB=-lm -lcurses

#Use this for BSD 4.2
#CFLAGS= -O -DBSD42
#LIB=-lm -lcurses -ltermcap

#Use this for BSD 4.3
CFLAGS= -O -DBSD43
LIB=-lm -lcurses -ltermcap

#Use this for system III (XENIX)
#CFLAGS= -O -DSYSIII
#LIB=-lm -lcurses -ltermcap

#Use this for separate I and D space
#ID=-i
ID=

$(name):	sc.o lex.o gram.o interp.o cmds.o
	cc ${CFLAGS} ${ID} sc.o lex.o gram.o interp.o cmds.o ${LIB} -o $(name)

diff_to_sc:	diff_to_sc.c
	cc ${CFLAGS} -o dtv diff_to_sc.c

lex.o:	sc.h y.tab.h gram.o
	cc ${CFLAGS} ${SIMPLE} -c lex.c

interp.o:	sc.h

sc.o:	sc.h

gram.o:	sc.h y.tab.h

cmds.o: cmds.c sc.h

y.tab.h:	gram.y

gram.o:	sc.h y.tab.h gram.c
	cc ${CFLAGS} -c gram.c
	sed<gram.y >experres.h -f eres.sed;sed < gram.y > statres.h -f sres.sed

gram.c:	gram.y
	yacc -d gram.y; mv y.tab.c gram.c

clean:
	rm -f *.o *res.h y.tab.h $(name) debug core gram.c 

shar: ${SRC1} ${SRC2} ${DOCS}
	shar -v -c ${DOCS} ${SRC2} > sc.shar.1
	shar -v -c ${SRC1} > sc.shar.2

lint: sc.h sc.c lex.c gram.c interp.c cmds.c
	lint ${CFLAGS} ${SIMPLE } sc.c lex.c gram.c interp.c cmds.c -lcurses -lm

$(name).1:	sc.doc
	sed -e s/pname/$(name)/g sc.doc >  $(name).1

$(name).man:	$(name).1
	-mv $(name).man $(name).mold
	nroff -man $(name).1 > $(name).man

install: $(EXDIR)/$(name)

inst-man: $(MANDIR)/$(name).1

$(EXDIR)/$(name): $(name)
	mv $(EXDIR)/$(name) $(EXDIR)/$(name).old
	strip $(name)
	cp $(name) $(EXDIR)

$(MANDIR)/$(name).1: $(name).1
	cp $(name).1 $(MANDIR)
