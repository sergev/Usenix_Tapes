#	Makefile	4.5	83/07/01
#
CFLAGS = -O

all:	sh

cp:	sh
	cp sh /bin/sh
	rm sh *.o

cmp:	sh
	cmp sh /bin/sh
	rm sh *.o

sh:	setbrk.o
sh:	blok.o stak.o defs.o
sh:	cmd.o fault.o main.o proc.o
sh:	word.o string.o name.o args.o
sh:	xec.o service.o error.o io.o
sh:	print.o macro.o expand.o
sh:	ctype.o msg.o
sh:	hash.o hashserv.o profile.o echo.o test.o
sh:	pwd.o func.o
blok.o:		brkincr.h
fault.o:	brkincr.h
main.o:		brkincr.h
stak.o:		brkincr.h

sh:
	cc -o sh *.o

install:
	install -s sh $(DESTDIR)/bin
clean:
	rm -f sh *.o
