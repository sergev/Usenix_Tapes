RTS = -DRTS
IMPLCD = 1
SLASH = 1
CFLAGS = -n -s -O
DEFS = defs.h mac.h mode.h name.h stak.h brkincr.h ctype.h
M = Makefile

sh:	args.o
sh:	blok.o
sh:	builtin.o
sh:	cmd.o
sh:	ctype.o
sh:	error.o
sh:	expand.o
sh:	extend.o
sh:	fault.o
sh:	io.o
sh:	macro.o
sh:	main.o
sh:	msg.o
sh:	name.o
sh:	print.o
sh:	service.o
sh:	setbrk.o
sh:	stak.o
sh:	string.o
sh:	word.o
sh:	xec.o
	cc -o sh -n -s *.o

.c.o:;	cc -O -c $(RTS) -DIMPLCD=$(IMPLCD) -DSLASH=$(SLASH) $*.c

args.o:	$M $(DEFS)

blok.o:	$M $(DEFS)

builtin.o:	$M

cmd.o:	$M $(DEFS) sym.h

ctype.o:	$M $(DEFS)

error.o:	$M $(DEFS) extend.h

expand.o:	$M $(DEFS)

extend.o:	$M $(DEFS) extend.h

fault.o:	$M $(DEFS)

io.o:	$M $(DEFS) dup.h

macro.o:	$M $(DEFS) sym.h extend.h

main.o:	$M $(DEFS) dup.h sym.h timeout.h extend.h

msg.o:	$M $(DEFS) sym.h

name.o:	$M $(DEFS)

print.o:	$M $(DEFS)

service.o:	$M $(DEFS) extend.h

setbrk.o:	$M $(DEFS)

stak.o:	$M $(DEFS)

string.o:	$M $(DEFS)

word.o:	$M $(DEFS) sym.h

xec.o:	$M $(DEFS) extend.h

