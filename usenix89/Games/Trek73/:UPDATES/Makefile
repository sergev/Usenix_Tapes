#
# Makefile for TREK73
#

#
# Select which operating system you are using.
# Acceptable flags are BSD and SYSV.
#
OS =	BSD

#
# Select whether you wish to use symbolic debugger or not.
# NOTE: UNIX System V Release 2.0 cannot do profiling on programs
# compiled with -g.  Also, sdb will not be very useful if the
# symbols are stripped from the load module. (See STRIP)
#SDB =	-g
SDB =

#
# Select whether code optimization is to be done.
OPT =	-O
#OPT =

#
# Select whether profiling is to be done.
# NOTE: In System V Relase 2.0, this is incompatible with
# both SDB and STRIP.
# PROF =	-p
PROF =

#
# Select whether or not the load module is to be stripped.
# This is incompatible with both SDB and PROF.
# Note: In BSD Unix, this option should always be blank
# STRIP =	-s
STRIP =

#
# Select whether or not tracing mode is to be turned on.
# This is useful when testing new strategies.
TRACE =	-DTRACE


DEFINES = -D${OS}
CFLAGS = ${DEFINES} ${SDB} ${OPT} ${PROF} ${STRIP}

T73OBJECTS = cmds1.o cmds2.o cmds3.o cmds4.o damage.o dist.o endgame.o\
	enemycom.o firing.o globals.o init.o main.o misc.o mission.o\
	moveships.o parseopts.o save.o ships.o special.o strat1.o\
	subs.o vers.o

T73CFILES = cmds1.c cmds2.c cmds3.c cmds4.c damage.c dist.c endgame.c\
	enemycom.c firing.c globals.c init.c main.c misc.c mission.c\
	moveships.c parseopts.c save.c ships.c special.c strat1.c\
	subs.c vers.c

BPVOBJECTS = bpv.o ships.o

BPVFILES = bpv.c ships.c

DYOOBJECTS = shipyard.o

DYOFILES = shipyard.c

HEADS= structs.h defines.h externs.h

RDIST = ${T73CFILES} ${BPVFILES} ${DYOFILES} ${HEADS}

LIBS= -lm

FLUFF = parsit.o make.out errs core lint.errs a.out tags\
	shar.1 shar.2 shar.3 shar.4

BINS = trek73 bpv shipyard


all:	trek73 bpv shipyard

trek73:	${T73OBJECTS} parsit.o
	cc ${CFLAGS} ${T73OBJECTS} parsit.o ${LIBS}
	install a.out trek73

bpv:	${BPVOBJECTS}
	cc ${CFLAGS} ${BPVOBJECTS}
	install a.out bpv

shipyard:	${DYOOBJECTS}
	cc ${CFLAGS} ${DYOOBJECTS} -lm
	install a.out shipyard

${T73OBJECTS}: ${HEADS}

${BPVOBJECTS}: ${HEADS}

${DYOOBJECTS}: ${HEADS}

parsit.o: parsit.c
	cc -O -c parsit.c

tags: ${T73CFILES}
	ctags ${T73CFILES} ${HEADS}

count:
	wc ${T73CFILES} ${HEADS}

lint:
	lint -abchx ${DEFINES} ${T73CFILES} > lint.errs

clean:
	rm -f ${BINS} ${T73OBJECTS} ${DYOOBJECTS} ${BPVOBJECTS} ${FLUFF}

shar: shar.1 shar.2 shar.3 shar.4

shar.1: Makefile defines.h externs.h options.h structs.h cmds1.c cmds2.c
	shar Makefile defines.h externs.h options.h structs.h cmds1.c cmds2.c >$@

shar.2: cmds3.c cmds4.c damage.c dist.c endgame.c enemycom.c
	shar cmds3.c cmds4.c damage.c dist.c endgame.c enemycom.c >$@

shar.3: firing.c globals.c init.c main.c misc.c mission.c bpv.c shipyard.c
	shar firing.c globals.c init.c main.c misc.c mission.c bpv.c shipyard.c >$@

shar.4: moveships.c parseopts.c parsit.c save.c ships.c special.c strat1.c subs.c vers.c 
	shar moveships.c parseopts.c parsit.c save.c ships.c special.c strat1.c subs.c vers.c >$@ 
