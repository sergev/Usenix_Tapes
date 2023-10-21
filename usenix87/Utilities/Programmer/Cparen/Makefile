CFLAGS = -g -Dindex=strchr -Drindex=strrchr
announce: cparen
	: cparen is built
cparen.cat: cparen.1
	nroff -man cparen.1 >cparen.cat
cparen: cparen.o parse.o lex.o hid.o
	cc -o cparen cparen.o lex.o parse.o hid.o -ll
cparen.o: cparen.c cparen.h parse.c
parse.o: parse.c cparen.h
lex.o: lex.c cparen.h parse.c
hid.o: hid.c cparen.h
parse.c: parse.y
	-rm -f parse.c
	: There should be 7 shift/reduce conflicts
	yacc -d parse.y
	mv y.tab.c parse.c
	chmod a-w parse.c
lex.c: lex.l
	-rm -f lex.c
	lex lex.l
	mv lex.yy.c lex.c
	chmod a-w lex.c
clean:
	-rm -f parse.c lex.c y.tab.h hid.o lex.o parse.o cparen.o
