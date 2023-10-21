#
# Makefile for uutty login daemon.  To build uutty, just type:
#	make
# To install uutty and the manual page, type:
#	make install
# It's easiest to test if you create an inittab entry like:
#	t43:234:respawn:uutty /dev/tty43 -B96 -l -d8 2>>/user/aud/tty43 
# where /dev/tty43 is a null-modem link to a port on the same or
# another Unix machine.  It should be tested in two directions.
# First, run it with no getty on the other end, and use cu from
# the other end to call up uutty.  After 1 or 2 CR or LF characters,
# it should log you in.  Next, start up getty or uutty on the other
# end, and use cu to connect from this end.  After CR or LF, uutty
# should get out of the way and let the other end log you in.  
#
CFL=	-DSYS5 -I.
CC=	cc $(CFL) -c
D=	uutty.8
H=	*.h 
L=	cc $(CFL) -o
O=	args.o awrite.o checkid.o copy.o data.o dbg.o die.o exec.o \
	fillutmp.o findutmp.o gestate.o getime.o help.o lastfield.o \
	lockname.o lockup.o lockwait.o main.o makeraw.o makesane.o \
	nextbyte.o opendev.o option.o pread.o pswd.o pwrite.o \
	restdev.o resync.o sendbrk.o shprompt.o sig.o slowly.o st.o \
	talk.o unlock.o  
S1=	args.c awrite.c checkid.c copy.c data.c dbg.c die.c \
	exec.c fillutmp.c findutmp.c gestate.c getime.c help.c 
S2=	lastfield.c lockname.c lockup.c lockwait.c \
	main.c makeraw.c makesane.c nextbyte.c opendev.c option.c pread.c \
	pswd.c pwrite.c restdev.c resync.c sendbrk.c shprompt.c \
	sig.c slowly.c st.c talk.c unlock.c  
S=	$(S1) $(S2)
U1=	README Makefile install uutty.8 uutty.h dbg.h $(S1)
U2=	$(S2)
#
# How to compile most of the modules:
# 
.c.o:	$*.c $H;	$(CC) $*.c
#
progs:		uutty
lints:		uutty.L
uutty:		$O;		$L uutty $O
#
# Libraries:
#
install:	/etc/uutty /usr/man/man8/uutty.8
/etc/uutty:	uutty;		Cp uutty /etc/uutty
/usr/man/man8/uutty.8: uutty.8;	Cp uutty.8 /usr/man/man8/uutty.8
#
# Some modules that need special treatment:
#
dbg.o:		dbg.c dbg.h;		$(CC) dbg.c
makeraw.o:	makeraw.c dbg.h;	$(CC) makeraw.c
makesane.o:	makesane.c dbg.h;	$(CC) makesane.c
st.o:		st.c;			cc -c st.c
#
# Run lint on the whole mess:
#
uutty.L:	$H $S;	lint $(CFL) $S >uutty.L
#
# Build the distribution files:
#
uutty.dist:	uutty.dist1 uutty.dist2
uutty.dist1:	uutty.cover1 uutty.shar1;	cat uutty.cover1 uutty.shar1 >uutty.dist1
uutty.dist2:	uutty.cover2 uutty.shar2;	cat uutty.cover2 uutty.shar2 >uutty.dist2
#
# Build the shell archives:
#
uutty.shar1:	$(U1);	shar $(U1) >uutty.shar1
uutty.shar2:	$(U2);	shar $(U2) >uutty.shar2
#
