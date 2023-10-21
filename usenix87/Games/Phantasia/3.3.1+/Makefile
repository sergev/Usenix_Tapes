# Makefile for Phantasia 3.3.1 (modifed by Chris Robertson, Sept. 1985)
#
# NOTE:	It is important to run 'setup' before trying to run 'phantasia'.
#	'make install' does this for you.

RM	= -rm
SHAR	= shar
SHARDES	= /tmp
SHELL	= /bin/sh
LIBS	= -lm -lcurses

# NOTE -- you MUST use separate i/d space on small machines (use '-i')
CFLAGS	= -O

# FIGHT stuff is separate because fight.c ran the optimizer out of space on
# at least one C compiler I encountered, so things are set up to allow
# everything but fight.c to be optimized if necessary --Chris Robertson
FIGHTFLAGS	= -O

# set PLOTDEVICE to "> /dev/whatever" as necessary -- for drawing map
PLOTDEVICE	= | lp -s

# DEST is where the program and its support files reside
DEST	=	/usr/local/games/lib/phantasia

# BIN is where the accessible-to-everyone binary will live
BIN	=	/usr/local/games

# The following are program constants which are implementation dependent.
#
# PATH is the same as $DEST.  Note that there is no closing quote here.
# WIZARD is the login of the one who will clean up things.
# UID is the uid of game wizard.
# ACCESS is fopen() access to open a file for writing, without
#	clearing the file,  e.g. "a", or "r+".  (Use "r+" if you have it.)
# define OK_TO_PLAY to restrict playing access.  Also see function ok_to_play()
#	in main.c, and tailor to your own needs.
# define ENEMY to include code for checking of a 'hit list' of resricted 
#	accounts barred from playing.  The list of logins goes in the file
#	'enemy'.
# define SHELL to enable shell escapes (might not want then if games are played
#   in a restricted environment)
# define NOVOID if your compiler does not support the "void" type (urk!)
# define BSD41 for 4.1bsd
# define BSD42 for 4.2bsd -- also for V7 since they are very similar, but you
#   will need to comment out the signal calls with SIGTSTP, SIGTTIN, and
#   SIGTTOU in func1.c, because V7 doesn't have them
# define SYS3 for System III, or similar
# define SYS5 for System V
# define WORM to include wormholes; omit them to save a little space 
# define SMALL on systems with 16-bit address space
# NOTE -- you MUST use separate i/d space on small machines
# If you don't have 'strchr()', put '-Dstrchr=index' below.

FLAGS	= -DPATH=\"${DEST} -DWIZARD=\"games\" -DUID=98 -DACCESS=\"a\"\
		  -DENEMY -DSHELL -DSYS5 -DWORM -Dstrchr=index

OFILES	= main.o func0.o func1.o func2.o func3.o
CFILES	= main.c func0.c func1.c func2.c func3.c

all:	phantasia setup convert phant.help

# The link is done for convenience -- it's shorter to type --CMR
install:	all
		if [ ! -f ${DEST}/characs ] ;then cat /dev/null > ${DEST}/characs;fi
		if [ ! -f ${DEST}/enemy ] ;then cat /dev/null > ${DEST}/enemy;fi
		if [ ! -f ${DEST}/gold ] ;then cat /dev/null > ${DEST}/gold;fi
		if [ ! -f ${DEST}/goldfile ] ;then cat /dev/null > ${DEST}/goldfile;fi
		if [ ! -f ${DEST}/lastdead ] ;then cat /dev/null > ${DEST}/lastdead;fi
		if [ ! -f ${DEST}/mess ] ;then cat /dev/null > ${DEST}/mess;fi
		if [ ! -f ${DEST}/scoreboard ] ;then cat /dev/null > ${DEST}/scoreboard;fi
		if [ ! -f ${DEST}/void ] ;then cat /dev/null > ${DEST}/void;fi
		setup
		cp phantasia ${BIN}
		cp convert phant.help monsters ${DEST}
		cp ${DEST}/characs ${DEST}/characs.orig
		cd ${DEST}; ./convert; mv newcharacs characs
		chmod 4711 ${BIN}/phantasia

phantasia:	${OFILES} fight.o pathdefs.o
		cc ${CFLAGS} ${OFILES} fight.o pathdefs.o ${LIBS} -o phantasia

setup:	phant.h pathdefs.o
		cc ${CFLAGS} ${FLAGS} pathdefs.o setup.c ${LIBS} -o setup

convert:
		cc ${CFLAGS} ${FLAGS} convert.c -o convert

# the flags below on tbl and nroff are to make a line printable version
# Note:	if your 'make' uses the cshell be sure to use >! not >

phant.help:	phant.nr
		tbl -TX phant.nr | nroff -Ttn300 -man >! phant.help

map:	map.o
		cc -n map.o -lplot -o map
		map | plot ${PLOTDEVICE}

lint:
		lint ${CFLAGS} ${FLAGS} func?.c main.c fight.c pathdefs.c ${LIBS}

clean:
		${RM} -f *.o phant.help phant.pt?

# Note:	if your 'make' uses the cshell be sure to use >! not >

shar:
		${SHAR} README phant.nr func0.c >! phant.pt1
		${SHAR} Makefile* fight.c func1.c phant.h >! phant.pt2
		${SHAR} func2.c func3.c main.c monsters pathdefs.c setup.c >! phant.pt3
		${SHAR} map.c convert.c >! phant.pt4

###

main.o:	phant.h main.c
		cc -c ${CFLAGS} ${FLAGS} main.c

fight.o:	phant.h fight.c
		cc -c ${FIGHTFLAGS} ${FLAGS} fight.c

func0.o:	phant.h func0.c
		cc -c ${CFLAGS} ${FLAGS} func0.c

func1.o:	phant.h func1.c
		cc -c ${CFLAGS} ${FLAGS} func1.c

func2.o:	phant.h func2.c
		cc -c ${CFLAGS} ${FLAGS} func2.c

func3.o:	phant.h func3.c
		cc -c ${CFLAGS} ${FLAGS} func3.c

pathdefs.o:	phant.h pathdefs.c
		cc -c ${CFLAGS} ${FLAGS} pathdefs.c
