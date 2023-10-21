#
# Makefile.b -	makefile for binary distribution
#
#	The following macros must be redefined for your installation.
#
#	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
#
GROUP	=	sorcerer
OWNER	=	wizard
BINDEST	=	/usr/games
LIBDEST	=	/usr/games/lib
WIZARD_UID=	666
#
#	The following need not be changed.
#

OBJS	=	config.o
SRCS	=	config.c
OS_VERS =	BSD42
PM_ROLL	=	$(LIBDEST)/pm_roll
PM_USER	=	$(LIBDEST)/pm_user
DEFS	=	-DPM_ROLL=\"$(PM_ROLL)\" -DPM_USER=\"$(PM_USER)\" \
		-D$(OS_VERS) -DWIZARD_UID=$(WIZARD_UID)
CFLAGS	=	-O $(DEFS)
LDFLAGS	=	-o pm
CRLIB	=	-lcurses
TERMLIB	=	-ltermlib
PMLIB	=	pm.a

all:		pm

install:	all pm_roll pm_user
		cp pm $(BINDEST)
		chgrp $(GROUP) $(BINDEST)/pm
		chgrp $(GROUP) $(PM_ROLL) $(PM_USER)
		chown $(OWNER) $(BINDEST)/pm
		chown $(OWNER) $(PM_ROLL) $(PM_USER)
		chmod 4711 $(BINDEST)/pm

pm:		$(OBJS)
		ranlib $(PMLIB)
		cc $(LDFLAGS) $(OBJS) $(PMLIB) $(CRLIB) $(TERMLIB)

$(OBJS):	Makefile.b

Makefile.b:

pm_roll:
		> $(PM_ROLL)
pm_user:
		> $(PM_USER)

clean:
		rm -f pm config.o

remove:		clean
		rm -f $(PM_ROLL) $(PM_USER)
