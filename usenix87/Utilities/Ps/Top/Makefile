# Makefile for "top", a top 10 process display for Unix
#
# This makefile is for top, version 2.1
#
# Written by William LeFebvre, Rice University graduate student

# installation information:
#	OWNER	- name (or uid) for the installed executable's owner
#	GROUP	- group name (or gid) for the installed executable's group
#	MODE	- mode for the installed executable (should start with a 0)
#	BINDIR	- directory where the executable should live
#	MANDIR	- directory where the manual page should live
#	MAN	- troff macros for manual pages
#	TROFF	- most appropriate troff command

OWNER  = phil
GROUP  = staff
MODE   = 755
BINDIR = /usr/local
MANDIR = /usr/man/manl
MAN    = man
TROFF  = troff

# Values for the two defaults in "top":
#	TOPN	- default number of processes to display
#	DELAY	- default delay between updates
#
# set TOPN to -1 to indicate infinity (so that top will display as many
# as the screen will hold).

TOPN = 10
DELAY = 5

TABLE =
# Top maintains an internal hash table for translating uid to username.
# This hash table must be big enough to hold every name in /etc/passwd.
# It is possible, but not necessary, to specify the hash table size in
# this Makefile.  Just uncomment the following line and provide a number.
#TABLE = -DTable_size=

TARFILES = README Changes Makefile top.c commands.c display.c kernel.c \
	   screen.c getopt.c \
	   boolean.h layout.h screen.h top.h top.local.h bzero.c \
	   sigconv.awk top.man
OBJS = top.o commands.o display.o kernel.o screen.o getopt.o

# Top uses the preprocessor variables "sun" and "pyr" for specific changes
# required by Suns and Pyramids.  No changes to "CFLAGS" are required for
# these architectres.
CFLAGS = -O
# To make a version for 4.1, comment out the previous line and
# uncomment the following two lines:
#CFLAGS = -DFOUR_ONE -O
#OBJS = top.o commands.o display.o kernel.o screen.o getopt.o bzero.o

all: top top.1

top: $(OBJS)
	cc $(CFLAGS) -o top $(OBJS) -ltermcap -lm

top.o: top.c Makefile
	cc -c $(CFLAGS) $(TABLE) -DDefault_TOPN=$(TOPN) -DDefault_DELAY=$(DELAY) top.c

# include file dependencies
top.o: boolean.h layout.h screen.h top.h top.local.h
commands.o: sigdesc.h
display.o: boolean.h layout.h screen.h top.h
kernel.o: top.local.h
screen.o: boolean.h screen.h

# automatically built include file
sigdesc.h: sigconv.awk /usr/include/signal.h
	awk -f sigconv.awk /usr/include/signal.h >sigdesc.h

# top.1 is built by combining the actual text with the default information
top.1: top.man Makefile
	echo '.nr N' $(TOPN) > top.1
	echo '.nr D' $(DELAY) >>top.1
	cat top.man >>top.1

top.cat: top.1
	nroff -$(MAN) top.1 | cat -s >top.cat

troff: top.1
	$(TROFF) -man top.1

tar:
	rm -f top.tar
	tar cvf top.tar $(TARFILES)

clean:
	rm -f *.o top top.cat top.tar top.1 core

install: top top.1
	install -s -o $(OWNER) -m $(MODE) -g $(GROUP) top $(BINDIR)
	install -c top.1 $(MANDIR)
