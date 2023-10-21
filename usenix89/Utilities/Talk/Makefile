#	Makefile for Sys V talk
#

BIN=/usr/lbin

OBS= talk.o beep.o erasechar.o doupdate.o idlok.o nodelay.o wnoutrefresh.o \
	cbreak.o flushinp.o

talk:	$(OBS)
	cc -O -o talk $(OBS) -lcurses -ltermcap -lx

install:
	cp talk $(BIN)
	chmod 2711 $(BIN)/talk
	chown bin $(BIN)/talk
	chgrp bin $(BIN)/talk


clean:
	rm -f *.o talk a.out core

compress:
	compress *.c *.h

uncomp:
	uncompress *.c.Z *.h.Z
