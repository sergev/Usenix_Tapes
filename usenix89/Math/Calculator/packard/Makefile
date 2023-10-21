#
# makefile for ic
#
CFLAGS=-O
OFILES=gram.o lex.o symbol.o \
	expr.o main.o func.o builtin.o\
	util.o

ic: $(OFILES)
	cc $(CFLAGS) -o ic $(OFILES) -lm

clean:
	rm -f $(OFILES) gram.c lex.c y.tab.h ic
	
gram.c: gram.y
	yacc -d gram.y
	mv y.tab.c gram.c

lex.c: lex.l
	lex lex.l
	mv lex.yy.c lex.c
builtin.o: ic.h
expr.o: ic.h
expr.o: y.tab.h
func.o: ic.h
gram.o: ic.h
lex.o: ic.h
lex.o: y.tab.h
main.o: ic.h
symbol.o: ic.h
