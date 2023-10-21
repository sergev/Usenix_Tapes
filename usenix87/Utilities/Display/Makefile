# makefile for 'display'
# vix 15dec86 [stolen from 'which']

# start changing
CFLAGS		=	-O
DEST_DIR	=	/usr/local/bin
CURSES_LIBS	=	-lcurses -ltermcap
MAN_DIR		=	/usr/man/manl
MAN_SUFFIX	=	l
SHAR_ARGS	=	-b -c -v
# stop changing

SHAR_SOURCES	=	README display.man Makefile display.c
SHELL		=	/bin/sh

all		:	display

display		:	display.c
			cc $(CFLAGS) -o $@ display.c $(CURSES_LIBS)
			strip display

install		:	all
			mv display $(DEST_DIR)
			chmod 775 $(DEST_DIR)/display
			cp display.man $(MAN_DIR)/display.$(MAN_SUFFIX)

display.shar	:	$(SHAR_SOURCES)
			shar $(SHAR_ARGS) $(SHAR_SOURCES) > $@
