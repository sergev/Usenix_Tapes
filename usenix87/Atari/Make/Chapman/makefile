make.prg : d:mstring.o d:make.o
	link68.prg [u] d:make.68k=d:gemstart,d:make,d:mstring,d:ctype,d:osbind,d:gemlib,d:libf
	relmod.prg d:make
	rm.prg d:make.68k
	cp d:make.prg make.prg
*.o : *.c d:mstring.h
	CP68.PRG $*.c $*.i
	c068.prg $*.i $*.1 $*.2 $*.3 -f
	rm.prg $*.i
	c168.prg $*.1 $*.2 $*.s
	rm.prg $*.1
	rm.prg $*.2
	as68.prg -l -u -f d: $*.s
	rm.prg $*.s
d:make.o : d:make.c d:mstring.h

d:mstring.o : d:mstring.c d:mstring.h

