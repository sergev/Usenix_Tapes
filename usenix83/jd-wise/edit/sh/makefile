CFLAGS = -n -s -O

install:	sh
	cp sh /usr/lib/edit/sh
	rm sh *.o

sh:	setbrk.o
sh:	builtin.o blok.o stak.o
sh:	cmd.o fault.o main.o
sh:	word.o string.o name.o args.o
sh:	xec.o service.o error.o io.o
sh:	print.o macro.o expand.o
sh:	ctype.o msg.o
sh:	debug.o
blok.o:		brkincr.h
fault.o:	brkincr.h
main.o:		brkincr.h
stak.o:		brkincr.h

sh:;		cc -o sh -n -s *.o

.c.o:;	cc -O -c $<
.c.s:;	cc -O -S -o $*.s $<
.s.o:;	as - -o $*.o /usr/include/sys.s $<
