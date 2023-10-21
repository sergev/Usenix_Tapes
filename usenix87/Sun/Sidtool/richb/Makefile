#
#  Makefile for the SID tool.
#  Written by Rich Burridge - Sun Australia, June 1986.
#
#  Version 1.5
#
#  No responsibility is taken for any errors inherent either in the comments
#  or the code of this program, but if reported to me then an attempt will
#  be made to fix them.
#
BINARIES      = animate sidtool
BINDIR        = ../release
CCFLAGS       = -c -O
LDFLAGS       =
OBJS          = sid_blt.o  sid_main.o  sid_stuff.o  sid_sun.o
SRCS          = sid_blt.c  sid_main.c  sid_stuff.c  sid_sun.c
HDRS          = bltstuff.h exceptions.h sidtool.h
LIBS          = -lsuntool -lsunwindow -lpixrect
#
#  The following commands are declared.
#
all:            $(BINARIES)
		animate
		touch sidtool.hs
		chmod 777 sidtool.hs
		echo -n "" >/dev/tty
#
#  General Makefile stuff.
#
clean:
		rm -f animate sidtool sidtool.animate *.o core
lint:
		lint $(SRCS) /usr/lib/lint/llib-lpixrect.ln
#
#  Sid Tool specific stuff.
#
animate:        animate.o
		cc $(LDFLAGS) -o animate animate.o

animate.o:      animate.c
		cc $(CCFLAGS) animate.c

sidtool:        sid_blt.o sid_main.o sid_stuff.o sid_sun.o
		cc $(LDFLAGS) -o sidtool $(OBJS) $(LIBS)

sid_blt.o:      sid_blt.c $(HDRS)
		cc $(CCFLAGS) sid_blt.c

sid_main.o:     sid_main.c $(HDRS)
		cc $(CCFLAGS) sid_main.c
 
sid_stuff.o:    sid_stuff.c $(HDRS)
		cc $(CCFLAGS) sid_stuff.c

sid_sun.o:      sid_sun.c $(HDRS)
		cc $(CCFLAGS) sid_sun.c
