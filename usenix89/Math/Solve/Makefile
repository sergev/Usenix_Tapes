OBJS = solve_lex.o solve_gram.o solve.o
SRCS = solve_gram.y solve_lex.l solve.c

CFLAGS = -O
YFLAGS = -d

solve: y.tab.h solve.h $(OBJS)
	cc  $(CFLAGS) $(OBJS) -o solve -ll -lm -lcurses -ltermlib

y.tab.h: solve_gram.o

solve.1 : solve.man
	nroff -man solve.man > solve.1

clean: 
	rm -f solve.o solve_gram.o solve_lex.o y.tab.h

pr:
	pr $(SRCS) | lprn
