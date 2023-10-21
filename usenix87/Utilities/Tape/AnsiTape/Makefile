# Makefile for ANSITAPE
#
# This program creates, reads, writes, and takes directory listings of
# ANSI-formatted tapes. 
#
# This program was developed at 
#
# HQDA Army Artificial Intelligence Center
# Pentagon Attn: DACS-DMA
# Washington, DC  20310-0200 
#
# Phone: (202) 694-6900
# E-mail:
#	Arpa:		merlin%hqda-ai.uucp@smoke.brl.arpa
#		alternate	dshayes@smoke.brl.arpa
#	UUCP:		...!seismo!sundc!hqda-ai!merlin
#
# Support?  You want support?  What do I look like, a software house?
# Mail me your gripes, comments, and bugs, and I'll try to keep
# up with them.
#
# THIS PROGRAM IS IN THE PUBLIC DOMAIN.



# Define AT MOST ONE of the following
# Define this if you want debugging.
#CCOPT	= -g
# Define this if you want compiler optimizations.
CCOPT	= -O


# Define this if you want to handle IBM EBCDIC tapes.
IBM	= -DEBCDIC


# Define this to be the maximum size of a file you are willing to
# read to determine the record length.  Files smaller than this
# are read twice:  once to find the max record length, and the second
# time when writing the tape.  Files larger than this just assume
# that the recordlength == blocklength.  Probably not a great
# assumption, but we have to assume something.
#
# If this is not defined, the ansitape.c uses 100K by default.
#READMAX	= 100000


# This is the directory where ansitape will go.
BIN	= /usr/local/bin


# This is the directory where you put your
# unformatted manual pages for commands.
CMDMAN	= /usr/man/man1

# This is the directory where you put your
# unformatted manual pages for file formats.
FILEMAN	= /usr/man/man5

# This is the directory where you put your
# FORMATTED manual pages for file formats.
# Define this to be /tmp if you don't keep
# formatted copies of the file format manual pages.
# Cron should clean it out eventually.
FILECAT	= /usr/man/cat5


all: ansitape man

ansitape: ansitape.c ansitape.h tables.o
	cc ${CCOPT} -o ansitape ${IBM} ${READMAX} ansitape.c tables.o
	mv ansitape ${BIN}

tables.o: tables.c

man: man1 man5

man1: ansitape.1
	cp ansitape.1 ${CMDMAN}

man5: ansitape.5.tbl
	tbl ansitape.5.tbl > ${FILEMAN}/ansitape.5
	tbl ansitape.5.tbl | nroff -man | colcrt > ${FILECAT}/ansitape.5
