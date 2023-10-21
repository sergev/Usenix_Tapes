LIBS=-lcurses -ltermlib
LIB=/usr/games/lib/reversi
DESTDIR=
BIN=/usr/games
MODE=u=rwx,g=rx,o=
CFLAGS=-O -DLIB=\"$(DESTDIR)$(LIB)

reversi: reversi.o
	cc -o reversi reversi.o $(LIBS)

install: reversi
	cp reversi $(DESTDIR)$(BIN)
	strip $(DESTDIR)$(BIN)/reversi
	chmod $(MODE) $(DESTDIR)$(BIN)/reversi
