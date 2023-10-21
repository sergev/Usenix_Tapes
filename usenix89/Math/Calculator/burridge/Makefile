#
#  Makefile for calctool, a desktop calculator for the Sun.
#
#  Copyright (c) Rich Burridge - SUN Australia - October 1986.
#
#  Version 1.0.
#
#  No responsibility is taken for any errors inherent either in the comments
#  or the code of this program, but if reported to me then an attempt will
#  be made to fix them.
#
BINARIES      = calctool
BINDIR        = .
CCFLAGS       = -c -O
LDFLAGS       =
OBJS          = calctool.o
SRCS          = calctool.c
HDRS          =
LIBS          = -lm -lsuntool -lsunwindow -lpixrect
#
#  The following commands are declared.
#
all:            $(BINARIES)

release:        $(BINARIES)
		strip $(BINARIES)
#
#  General Makefile stuff.
#
backup:
		cp calctool.c calctool.c~
clean:
		rm -f *.o core
lint:
		lint $(SRCS) -lpixrect
#
#  calc specific stuff.
#
calctool:       $(OBJS)
		cc $(LDFLAGS) -o calctool $(OBJS) $(LIBS)

calctool.o:     calctool.c $(HDRS)
		cc $(CCFLAGS) calctool.c
