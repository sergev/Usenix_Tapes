#
#  Makefile for the SID tool.
#  Written by Rich Burridge - Sun Microsystems Australia (Melbourne).
#
#  Version 2.1.  -  April 1987.
#
#  No responsibility is taken for any errors inherent either in the comments
#  or the code of this program, but if reported to me then an attempt will
#  be made to fix them.
#
BINDIR          = ./release
CFLAGS          = -O
LDFLAGS         =
OBJS            = sid_blt.o sid_main.o sid_stuff.o sid_sun.o
SRCS            = sid_blt.c sid_main.c sid_stuff.c sid_sun.c
HDRS            = bltstuff.h sidtool.h
LIBS            = -lsuntool -lsunwindow -lpixrect

all:            sidtool

install:        all
		install -c -s sidtool $(BINDIR)
		make animate
		cp README sidtool.maze sidtool.help	$(BINDIR)

animate:
		(cd images; make BINDIR=$(BINDIR) install)

clean:
		(cd images; make clean)
		rm -f *~ *.o core sidtool.animate sidtool.hs sidtool

lint:
		(cd images; make lint)
		lint $(SRCS) -lsuntool -lsunwindow -lpixrect

backup:
		(cd images; make backup)
		cp sid_main.c sid_main.c~
		cp sid_blt.c sid_blt.c~
		cp sid_stuff.c sid_stuff.c~
		cp sid_sun.c sid_sun.c~
		cp sidtool.h sidtool.h~

shar:
		makekit -s24k * images/*

sidtool:        $(OBJS)
		cc $(LDFLAGS) -o sidtool $(OBJS) $(LIBS)

sid_blt.o:      sid_blt.c $(HDRS)
sid_main.o:     sid_main.c $(HDRS)
sid_stuff.o:    sid_stuff.c $(HDRS)
sid_sun.o:      sid_sun.c $(HDRS)
