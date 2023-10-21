#
#  Makefile for the sidtool animate program.
#  Written by Rich Burridge - Sun Microsystems Australia (Melbourne).
#
#  Version 2.1.  -  April 1987.
#
#  No responsibility is taken for any errors inherent either in the comments
#  or the code of this program, but if reported to me then an attempt will
#  be made to fix them.
#
BINARIES        = animate
CFLAGS          = -O
LDFLAGS         =
OBJS            = animate.o
SRCS            = animate.c

all:            $(BINARIES)

install:        all
		animate
		(cd .. ; cp images/sidtool.animate $(BINDIR))

clean:
		rm -f *~ *.o core animate sidtool.animate

lint:
		lint $(SRCS)

backup:
		cp animate.c animate.c~

animate:        $(OBJS)
		cc $(LDFLAGS) -o animate $(OBJS)

animate.o:      animate.c
