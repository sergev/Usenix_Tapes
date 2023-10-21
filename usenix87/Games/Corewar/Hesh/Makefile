#
#	Makefile for Corewar system
#
#static	char	rcsid[] = "$Header: Makefile,v 7.0 85/12/28 14:35:38 ccohesh Dist $";
#
#	[cw by Peter Costantinidis, Jr. @ University of California at Davis]
#
#
#	If you are running under SysV, I am told that you would want to
#	uncomment the following commented lines.
#
#	.SUFFIXES: .rc
#	.rc:
#		rcasm $< $@

HDRS	=	Makefile cw.h
YSRC	=	rcasm.y
LSRC	=	rcasm.l
SRCS	=	getopt.c mars.c movmem.c msg.c print.c scheduler.c sum.c\
		vdisplay.c vstatus.c
OBJS	=	getopt.o mars.o movmem.o msg.o print.o scheduler.o sum.o\
		vdisplay.o vstatus.o
PROGS	=	bdwarf.rc dwarf.rc fibo.rc gemini.rc imp.rc\
		random.rc stomper.rc
#
# libs
#
CWLIB	=	cw.a
CRLIB	=	-lcurses
TERMLIB	=	-ltermlib
ALLLIBS	=	$(CWLIB) $(CRLIB) $(TERMLIB)

#
# It has been suggested that FLAGS should include "-DSYS5" when running
# under SYS5
#
FLAGS	=	#-DDEBUG
CFLAGS	=	-O $(FLAGS)
LDFLAGS =	-o $@
LINTARGS=	-ahxc $(DEFS) -DLINT
SHAR	=	shar
SHAROUT	=	cw.shar
ALLSRCS	=	$(HDRS) $(SRCS) unrc.c cw.6 $(YSRC) $(LSRC)
ALLEXES	=	rcasm unrc mars
ALL	=	READ_ME $(ALLSRCS) $(CWDOC) $(SHAR)
CI	=	ci -u

NAME =		mars

demo:		mars rcasm
#
#	You may want to comment out the "rcasm" lines if
#	running SysV.
#
		rcasm imp.rc imp
		rcasm dwarf.rc dwarf
		rcasm gemini.rc gemini
		rcasm stomper.rc stomper
		mars -p imp dwarf gemini stomper

install:	mars rcasm unrc cw.nr

cw.nr:		cw.6
		tbl cw.6 | nroff -man | col > cw.nr

mars:		$(CWLIB)
		cc $(LDFLAGS) $(ALLLIBS)

#
# RCS stuff
#
ci:		$(ALLSRCS)
		-$(CI) $?
		@touch ci
coall:
		co -l $(ALLSRCS)
update:
		ci -sDist -u -f$(VERS) $(ALLSRCS)
		@touch ci

all:		$(ALLEXES)

$(CWLIB):	$(OBJS)
		@ar rv $@ $?; ranlib $@

$(OBJS):	$(HDRS)

rcasm:		rcasm.o $(CWLIB) rcasm.h
		cc $(LDFLAGS) rcasm.o $(CWLIB) -ll

rcasm.o:	rcasm.c rcasm.h $(HDRS)

rcasm.h:	$(LSRC) $(HDRS)
		$(LEX) $(LFLAGS) $(LSRC)
		@mv lex.yy.c rcasm.h

rcasm.c:	$(YSRC) $(HDRS)
		$(YACC) $(YFLAGS) $(YSRC)
		@mv y.tab.c rcasm.c

unrc:		unrc.o $(CWLIB)
		cc -O $(LDFLAGS) unrc.o $(CWLIB)

unrc.o:		$(HDRS)

lint:
		lint $(LINTARGS) $(SRCS)

print:
		lpr -Pc -p Makefile $(ALLSRC)

tags:		$(HDRS) $(SRCS)
		ctags -u $?
		sort tags -o tags

wc:
		wc $(ALLSRCS)

export:
		/bin/sh $(SHAR) $(ALL) $(PROGS) > $(SHAROUT)

clean:
		rm $(OBJS) rcasm.c rcasm.h
