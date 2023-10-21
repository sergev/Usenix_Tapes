LIB=/usr/games/lib/reversi
DESTDIR=
BIN=/usr/games
MAN=/usr/man/man6
MODE=u=rwx,g=rx,o=
LIBS="-lcurses -ltermlib"
#	other libraries may be needed for non-4.2 installations

default: reversi .informed

reversi:
	cd src; \
	make LIB=$(LIB) DESTDIR=$(DESTDIR) BIN=$(BIN) LIBS=$(LIBS)

.informed:
	sh inform
	echo >.informed

allinstall: install libinstall

install:
	cd src; \
	make LIB=$(LIB) DESTDIR=$(DESTDIR) BIN=$(BIN) MODE=$(MODE) \
		LIBS=$(LIBS) install
	@echo Type '"make libinstall"' to install documentation, etc.

libinstall:
	cd lib; make LIB=$(LIB) MAN=$(MAN) DESTDIR=$(DESTDIR) install

clean:
	rm -f src/reversi src/reversi.o

shar: reversi.shar

reversi.shar: clean Makefile READ.ME inform src/* lib/*
	shar Makefile READ.ME inform src lib >reversi.shar
