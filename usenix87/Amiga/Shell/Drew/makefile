######################################################################
#
# Makefile to build Shell 2.06M
# by Steve Drew 30-May-87
#
######################################################################

OBJS	= run.o main.o comm1.o comm2.o execom.o set.o sub.o \
		globals.o rawconsole.o sort.o

INCL	= shell.h

Shell	: $(OBJS)
	ln  +q -m -o Shell $(OBJS) -lc

rawconsole.o : rawconsole.c $(INCL)
	cc    +IShell.syms rawconsole.c

run.o   : run.c $(INCL)
	cc    +HShell.syms run.c

main.o  : main.c $(INCL)
	cc    +IShell.syms main.c

comm1.o	: comm1.c $(INCL)
	cc    +IShell.syms comm1.c

comm2.o	: comm2.c $(INCL)
	cc    +IShell.syms comm2.c

set.o	: set.c $(INCL)
	cc    +IShell.syms set.c

sub.o	: sub.c $(INCL)
	cc    +IShell.syms sub.c

globals.o : globals.c $(INCL)
	cc    +IShell.syms globals.c

execom.o : execom.c $(INCL)
	cc    +IShell.syms execom.c
