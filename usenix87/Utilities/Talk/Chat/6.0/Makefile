###############################################################################
####		   Makefile for The Chat System by SLB                     ####
###############################################################################
CC=cc

###############################################################################
# The DESTDIR is the final location of the compiled chat system along with
#  All commands (cwho, ruser)
DESTDIR=/usr/local/bin

###############################################################################
# OWNER and GROUP are the names of the owner and group of the chat system
# (usually bin bin or whoever decides to oversee Chat)
OWNER=bin
GROUP=bin

###############################################################################
# MANDIR is where the manual page for chat is to be stored (if at all).
MANDIR=/usr/man/man1

###############################################################################
# Defined flags:
# If your system has TERMIO then define it, otherwise Chat assumes you have
#   CBREAK mode in sgtty.h
# Define ENEMY if you wish to be able to "squelch" users who abuse their
#   Chat privledges (see README).
# Define VOID if your system has the void type.
# Define INDEX if your system has INDEX insted of STRCHR
CFLAGS= -Os -DTERMIO -DINDEX -DENEMY -DVOID

OBJECTS= main.o cursub.o ttyset.o page.o readmsg.o send.o misc.o quit.o

all: chat cwho ruser

chat:   chat.h $(OBJECTS) defs.h
	$(CC) -Os $(OBJECTS) -o chat -ltermlib

cwho:	defs.h cwho.c
	$(CC) -Os cwho.c -o cwho

ruser:	defs.h ruser.c
	$(CC) -Os ruser.c -o ruser

lint:
	lint -bxa *.c

install:
	mv chat ${DESTDIR}
	chown ${OWNER} ${DESTDIR}/chat
	chgrp ${GROUP} ${DESTDIR}/chat
	chmod ug+s     ${DESTDIR}/chat
	mv cwho ${DESTDIR}
	mv ruser ${DESTDIR}
	chown ${OWNER} ${DESTDIR}/ruser
	chgrp ${GROUP} ${DESTDIR}/ruser
	chmod ug+s     ${DESTDIR}/ruser
	mv chat.1 ${MANDIR}

clean:
	rm -f ${OBJECTS}
