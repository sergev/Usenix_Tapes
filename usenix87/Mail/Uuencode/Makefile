CFLAGS	= -O

all:	uuencode uudecode
install:	all
	cp uuencode uudecode /usr/bin
	strip /usr/bin/uuencode
	strip /usr/bin/uudecode
	cp uuencode.1 /usr/man/man1/uuencode.1

uuencode:	uuencode.c	;	cc -o $@ $(CFLAGS) $?
uudecode:	uudecode.c	;	cc -o $@ $(CFLAGS) $?
