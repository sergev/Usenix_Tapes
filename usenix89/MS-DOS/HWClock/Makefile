CFLAGS=-Oiz

all:		setclock sysdate


setclock:	out.o setclock.c clock.h
		cc out.o setclock.c -o setclock

sysdate:	in.o sysdate.c clock.h
		cc in.o sysdate.c -o sysdate

in.o:		in.s
		cc -c in.s

out.o:		out.s
		cc -c out.s

install:	setclock sysdate
		/etc/install -f /etc sysdate
		/etc/install -f /etc setclock

clean:		
		rm -f sysdate setclock *.o
