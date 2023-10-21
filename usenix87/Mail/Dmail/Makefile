
#   V1.00 (first distribution)
#
#   Written by Matthew Dillon, distribution date 6 December 1985
#
#   (C)1985 by Matthew Dillon
#
#   This code is completely original.  I declare this code to be public
#   domain.  You may have this code as long as any redistributions
#   contain this and all other files in their entirety, especially these
#   comments.
#
#   Send bug reports and other gripes 
#
#   dillon@ucb-vax.berkeley.edu         ARPA NET
#   ...!ucbvax!dillon                   USENET
#
#   INSTRUCTIONS FOR COMPILING:
#
#   (1) Set DESTDIR & HELP_DIR to some global directory accessable to
#       everybody (or whatever).  Also set HELP_CHMOD and PROG_CHMOD
#       to the right thing if you don't like the defaults.
#
#   (2) Do ONE of the following:
#       make            -defaults to internal help file
#       make external   -external help file (executable is smaller)
#
#   (3) make install
#

CC = cc

CFLAGS      = 
DEST_DIR    =	../../bin
HELP_DIR    =	/usr/public
HELP_FILE   =	${HELP_DIR}/dmail.help
HELP_CHMOD  =	644
PROG_CHMOD  =	751

OBJECTS     =	globals.o main.o do_lists.o sub.o execom.o commands.o range.o \
		load_mail.o sendmail.o set.o help.o

HEADERS     =	dmail.h execom.h


internal:	dmkhelp	${OBJECTS}
	${CC} ${CFLAGS} -o dmail ${OBJECTS}

external:	${OBJECTS}
	${CC} ${CFLAGS} -c -DHELPFILE=\"${HELP_FILE}\" help.c
	${CC} ${CFLAGS} -o dmail ${OBJECTS}

${OBJECTS}:	${HEADERS}

help.o:		dmkhelp
	./dmkhelp > .dmkout
	${CC} ${CFLAGS} -c help.c

dmkhelp:	dmail.help dmkhelp.o
	${CC} ${CFLAGS} -o dmkhelp dmkhelp.o

clean:
	rm -f *.o make.out a.out core dmail dmkhelp .dmkout

install:
	cp dmail ${DEST_DIR}
	cp dmail.help ${HELP_DIR}
	chmod ${PROG_CHMOD} ${DEST_DIR}/dmail
	chmod ${HELP_CHMOD} ${HELP_DIR}/dmail.help

