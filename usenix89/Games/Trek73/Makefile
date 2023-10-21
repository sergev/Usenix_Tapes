#
# makefile for trek73
#

CFLAGS=-O
OBJECTS= cmds1.o cmds2.o cmds3.o cmds4.o dist.o endgame.o enemycom.o\
	firing.o globals.o main.o misc.o mission.o moveships.o\
	parseopts.o strat1.o subs.o
CFILES= cmds1.c cmds2.c cmds3.c cmds4.c dist.c endgame.c enemycom.c\
	firing.c globals.c main.c misc.c mission.c moveships.c\
	parseopts.c strat1.c subs.c
HEADS= structs.h defines.h
LIBS= -lm

all:	trek73

trek73:	${OBJECTS} parsit.o
	cc ${CFLAGS} ${OBJECTS} parsit.o ${LIBS}
	install a.out trek73

${OBJECTS}: ${HEADS}

parsit.o: parsit.c
	cc -O -c parsit.c

tags: ${CFILES}
	ctags ${CFILES} ${HEADS}

lint:
	lint -za ${CFILES} > lint.out

clean:
	rm -f ${OBJECTS} parsit.o trek73 make.out errs core lint.out a.out
