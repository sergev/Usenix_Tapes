#
# $Header: Makefile,v 1.10 86/12/17 19:36:12 mcooper Exp $
#
# Makefile for Game Regulator.
#

#
# GAMEDIR - This is where the symbolic links are to be placed.
#	    Should be /usr/games.
#
GAMEDIR = /usr/games

#
# HIDEDIR - Name of directory to hide the actual executables
#
HIDEDIR = $(GAMEDIR)/.hide

#
# MAN5 - Manual directory for gr.conf.5
# MAN8 - Manual directory for gr.8
#
MAN5 = /usr/usc/man/man5
MAN5 = /usr/usc/man/man8

# 
# OWNER - This is the user who owns HIDEDIR.
# GROUP - This group has read/execute permissions on HIDEDIR and owns
#	  the Game Regulator which is set[ug]id to OWNER/GROUP.
#	  Either the owner or the group must have read access to
#	  /dev/kmem.
#
OWNER = daemon
GROUP = kmem

#
# NAME - Name of Game Regulator in GAMEDIR.  I have it set to ".gr"
# 	 so it doesn't show up with ls(1) by default.
#
NAME = .gr

#
# GAMES - The games in the original GAMEDIR to be put under regulation.
#	  Currently set to the games distributed with 4.3BSD.
#
GAMES = aardvark adventure arithmetic backgammon battlestar bcd\
	bogdict boggle btlgammon canfield chess ching cribbage\
	doctor factor fish hangman hunt mille rain robots rogue\
	sail snake teachgammon trek worm worms wump zork

CFLAGS = -O

OBJS = gr.o logfile.o parse.o
SRCS = gr.c gr.h logfile.c parse.c

gr: $(OBJS)
	cc $(OBJS) -o gr

#
# Set "CFLAGS=-DDEBUG" if you want gr compiled with debug info.
#
debug: $(OBJS)
	cc -g $(OBJS) -o gr
	rm -f gr
	ln -s gr debug

.c.o:
	cc $(CFLAGS) -c $*.c

$(OBJS): gr.h

install: gr gr.conf.5 gr.8
	install -m 6751 -o $(OWNER) -g $(GROUP) gr $(GAMEDIR)/$(NAME)
	install -c gr.conf.5 $(MAN5)
	install -c gr.8 $(MAN8)

create:
	-mkdir $(HIDEDIR)
	chown $(OWNER) $(HIDEDIR)
	chgrp $(GROUP) $(HIDEDIR)
	chmod 770 $(HIDEDIR)
	cp gr.conf $(GAMEDIR)/lib
	touch $(GAMEDIR)/lib/gr.log
	chown $(OWNER) $(GAMEDIR)/lib/gr.conf $(GAMEDIR)/lib/gr.log
	chgrp $(GROUP) $(GAMEDIR)/lib/gr.conf $(GAMEDIR)/lib/gr.log
	chmod 664 $(GAMEDIR)/lib/gr.conf $(GAMEDIR)/lib/gr.log

dolink: dolink.sh
	sed -e s#HIDEDIR#$(HIDEDIR)# -e s#GAMEDIR#$(GAMEDIR)# \
		-e s#NAME#$(NAME)# dolink.sh > dolink
	chmod +x dolink
	dolink $(GAMES)

clean:
	rm -f *.o log dolink gr.shar debug

shar:
	shar README gr.8 gr.conf.5 Makefile gr.conf $(SRCS) dolink.sh > gr.shar
