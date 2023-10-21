CFLAGS=-O -i
OFILES=user.o ulex.o move.o display.o hasmove.o fini.o \
	minmax.o score.o edges.o corners.o count.o
SOURCE= makefile corners.c count.c display.c edges.c fini.c genedge.c \
	hasmove.c makeedge.y minmax.c move.c reversi.h score.c ulex.l user.y

reversi: $(OFILES)
	cc $(CFLAGS) -o reversi $(OFILES) -lcurses -ltermlib

reversi.shar: $(SOURCE)
	shar -c $(SOURCE) > reversi.shar

$(OFILES): reversi.h
user.o: user.c
user.c: user.y
	yacc -dv user.y
	mv y.tab.c user.c
ulex.o: ulex.c
ulex.c: ulex.l
	lex ulex.l
	mv lex.yy.c ulex.c
corners.o: corners.c
	$(CC) $(CFLAGS) -R -c corners.c

edges.o: edges.c edges.out
	$(CC) $(CFLAGS) -R -c edges.c
edges.out: makeedge genedge
	genedge | makeedge > edges.out
makeedge: makeedge.o
	$(CC) $(CFLAGS) -o makeedge makeedge.o
makeedge.o: makeedge.c
makeedge.c: makeedge.y
	yacc makeedge.y
	mv y.tab.c makeedge.c
genedge: genedge.o
	$(CC) $(CFLAGS) -o genedge genedge.o
genedge.o: genedge.c
