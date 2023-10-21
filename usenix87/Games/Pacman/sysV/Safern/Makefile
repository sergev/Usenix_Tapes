#
# Makefile:    makefile for pm
#
#      Tunables:
#              PM_ROLL -       pathname of score file
#              PM_USER -       pathname of user log file
#              OS_VERS -       operating system version
#                              (BSD29, BSD41, BSD42, BSD43, SYSIII, SYSV)
#              SEND    -       pathname of binary export directory
#
#      Please note: on system 5.2 and any systems whose curses does not
#      use termcap, you must comment out TERMLIB.
#
#      [pm by Peter Costantinidis, Jr. @ University of California at Davis]
#

#
# Files
#
HDRS    =       pm.h
OBJS    =       flsbuf.o init.o make_moves.o\
		misc.o monsters.o msg.o pm.o random.o\
		rip.o score.o screen.o timing.o warning.o
SRCS1   =       config.c flsbuf.c init.c make_moves.c\
		misc.c monsters.c msg.c
SRCS2   =       pm.c random.c rip.c score.c screen.c timing.c warning.c
SRCS    =       $(SRCS1) $(SRCS2)

#
# Configuration section
#
PM_ROLL	=       /y1/safern/bin/lib/pm_roll
PM_USER =       /y1/safern/bin/lib/pm_user
OS_VERS =       SYSV
DEFS	=       -DPM_ROLL=\"$(PM_ROLL)\" -DPM_USER=\"$(PM_USER)\" \
		-DWIZARD_UID=283
DEST	=       /y3/safern/bin/lib/xhesh

#
# Flags section
#
CFLAGS  =       -O -D$(OS_VERS)
LDFLAGS =       -o tester
LINTARGS =      -ahxc $(DEFS) -DLINT
CRLIB	=       -lcurses
TERMLIB	=       -ltermcap     # on ATT 5.2, this line must be commented out.

#
# Misc.
#
PMLIB	=       pm.a
PMDOC	=       pm.6
BEXPORT	=       READ_ME Makefile.b TODO $(PMLIB) $(PMDOC) config.c
ALL     =       READ_ME Makefile Makefile.b TODO $(HDRS) $(SRCS) $(PMDOC) shar
REST    =       READ_ME Makefile Makefile.b TODO $(HDRS) $(PMDOC) shar
EX1     =       $(REST) $(SRCS2)
EX2     =       $(SRCS1)

tester:         config.o $(OBJS)
		-mv tester tester.old
		cc $(LDFLAGS) $(OBJS) config.o $(CRLIB) $(TERMLIB)
		chmod 04711 tester

config.o:       $(HDRS)
		cc $(CFLAGS) $(DEFS) -c config.c

install:	pm pm_roll pm_user clean

clean:
		rm $(OBJS) *.old

pm:		tester
		-mv pm pm.old; mv tester pm
#		@strip pm
		>tester

$(OBJS):	$(HDRS)

tags:		$(HDRS) $(SRCS)
		ctags -u $?
		sort tags -o tags

pm_roll:
		> $(PM_ROLL)

pm_user:
		> $(PM_USER)

Print:		$(HDRS) $(SRCS)
		pr $? | print
		-touch Print

P_all:
		pr $(HDRS) $(SRCS) | print
		-touch Print

lint:
		lint $(LINTARGS) $(SRCS) -lcurses

count:
		wc $(HDRS) $(SRCS)

bexport:	$(PMLIB)
		cp $(BEXPORT) $(DEST)

export:
		/bin/sh shar $(EX1) > pm1.shar
		/bin/sh shar $(EX2) > pm2.shar

crypt:		pm.tar
		crypt < pm.tar > pm.tar.c

Save:		$(HDRS) $(SRCS)
		cp $? Makefile xsave

Cxref:		$(SRCS) $(HDRS)
		cxref $(SRCS) $(HDRS) > $@
