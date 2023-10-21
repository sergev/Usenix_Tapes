# Makefile for multibatch and uucast.
#
# Copyright (C) Shane McCarron, 1987.
#
# Note that the symbols used here are compatible with those of
# news 2.10.3 and beyond...

# Directory where news batch files are kept.
BATCHDIR = /mecc/spool/batch

# Directory with news software is kept.
LIBDIR = /mecc/bin/news

# Define WANTZ if you don't want uux completion notices from other systems.
DEFS =	-DBATCHDIR=\"$(BATCHDIR)\" -DLIBDIR=\"$(LIBDIR)\" -DWANTZ
CFLAGS = -O

FILES = multibatch multisend uucast

all: $(FILES)

multibatch: Makefile
	$(CC) $(CFLAGS) $(DEFS) multibatch.c -o multibatch

multisend: Makefile multisend.sh
	sed -e 's;LIBDIR;$(LIBDIR);g' < multisend.sh > multisend
	chmod +x multisend

uucast: Makefile uucast.h
	$(CC) $(CFLAGS) $(DEFS) uucast.c -o uucast
