#    Makefile for grotwin window manager on 24 x 80 type terminals

#    Enable next line to install for Sun
#    SUNFLAGS=-DSUN

grotwin: grotwin.o manager.o window.o update.o utmp.o
	cc -O -o grotwin grotwin.o manager.o window.o update.o utmp.o -lcurses -ltermlib

grotwin.o:	grotwin.c grotwin.h
		cc $(SUNFLAGS) -O -c grotwin.c

manager.o:	manager.c grotwin.h
		cc $(SUNFLAGS) -O -c manager.c

window.o:	window.c grotwin.h
		cc $(SUNFLAGS) -O -c window.c

update.o:	update.c grotwin.h
		cc $(SUNFLAGS) -O -c update.c

utmp.o:		utmp.c
		cc $(SUNFLAGS) -O -c utmp.c

man:
		nroff -man grotwin.1 > grotwin.man
