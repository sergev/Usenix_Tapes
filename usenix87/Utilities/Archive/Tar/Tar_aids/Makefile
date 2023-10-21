all:	targ tarl

targ:	targ.c
	cc $(CFLAGS) -o $@ $<

tarl:	tarl.c
	cc $(CFLAGS) -o $@ $<

# The 'echo' causes the "right thing" to happen on 4.[23]BSD systems.
DEST	= /usr/local/bin
MAN	= /usr/man/man1
install:	all
	cp targ tarl $DEST
	cp targ.1 $MAN/targ.1
	echo ".so man1/tarl.1" >$MAN/targ.1
