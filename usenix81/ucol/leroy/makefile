SOURCES = leroy.y lexroy.l vlist.c main.c subs.c lexio.c
CFLAGS=-O
YFLAGS=-dv

leroy:	y.tab.o lex.yy.o
	cc -i -o leroy -O y.tab.o lex.yy.o -lln plotsrc/plib -lF77 -lI77 -lm
	cp leroy /bin/leroy
	rm leroy

y.tab.o:	leroy.y vlist.c main.c subs.c
		yacc -dv leroy.y
		cc -c -O y.tab.c

lex.yy.o:	lexroy.l lexio.c
		lex lexroy.l
		cc -c -O lex.yy.c

listing:	$(SOURCES) makefile y.tab.h
	pr $? | vpr
	touch listing

clean:
		rm y.output y.tab.* lex.yy.*
