CC=	cc
CFLAGS=	-O -s
GAMEDIR= /usr/local/games
ALL=	Bsnake snake
CFILES=	Bsnake.c snake.c

all: $(ALL)
Bsnake: Bsnake.c
	$(CC) $(CFLAGS) $@.c -o $@ -lcurses
snake: snake.c
	$(CC) $(CFLAGS) $@.c -o $@ -lcurses

install: $(ALL)
	cp snake $(GAMEDIR)
	cp Bsnake ~bob/bin
clean:
	rm -f $(ALL)
