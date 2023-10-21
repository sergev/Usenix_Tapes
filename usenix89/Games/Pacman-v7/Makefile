#
# Makefile:	makefile for pm
#
#	Tunables:
#		PM_ROLL -	pathname of score file
#		PM_USER -	pathname of user log file
#		OS_VERS -	operating system version
#				(BSD29, BSD41, BSD42, BSD43)
#		SEND	-	pathname of binary export directory
#
#	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
#

#
# Files
#
HDRS	=	pm.h
OBJS	=	config.o flsbuf.o init.o make_moves.o\
		misc.o monsters.o msg.o pm.o random.o\
		rip.o score.o screen.o timing.o warning.o
SRCS1	=	config.c flsbuf.c init.c make_moves.c\
		misc.c monsters.c msg.c
SRCS2	=	pm.c random.c rip.c score.c screen.c timing.c warning.c
SRCS	=	$(SRCS1) $(SRCS2)
#
# Configuration section
#
PM_ROLL	=	/usfa/g01030/ccohesh001/pm_roll
PM_USER	=	/usfa/g01030/ccohesh001/pm_user
OS_VERS =	BSD42
DEFS	=	-DPM_ROLL=\"$(PM_ROLL)\" -DPM_USER=\"$(PM_USER)\" \
		-D$(OS_VERS) -DWIZARD_UID=16649
DEST	=	/usfa/g01030/ccohesh001/xhesh
#
# Flags section
#
CFLAGS	=	-O $(DEFS)
LDFLAGS	=	-o tester
LINTARGS=	-ahxc $(DEFS) -DLINT
CRLIB	=	-lcurses
TERMLIB	=	-ltermlib
#
# Misc.
#
PMLIB	=	pm.a
PMDOC	=	pm.6
BEXPORT	=	READ_ME Makefile.b TODO $(PMLIB) $(PMDOC) config.c
ALL	=	READ_ME Makefile Makefile.b TODO $(HDRS) $(SRCS) $(PMDOC) shar
REST	=	READ_ME Makefile Makefile.b TODO $(HDRS) $(PMDOC) shar
EX1	=	$(REST) $(SRCS2)
EX2	=	$(SRCS1)

tester:		$(PMLIB)
		-mv tester tester.old
		cc $(LDFLAGS) $(PMLIB) $(CRLIB) $(TERMLIB)
		chmod 04711 tester

install:	pm pm_roll pm_user clean

$(PMLIB):	$(OBJS)
		ar rv $@ $?
		ranlib $@
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
		pr -l60 -w132 $? | lpr -d "-tpm"
		-touch Print

P_all:
		pr -l60 -w132 Tags $(HDRS) $(SRCS) | lpr -mc -d "-tpm"
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
