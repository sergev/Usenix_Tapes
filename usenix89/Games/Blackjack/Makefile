# If you can edit /etc/services, define ROOTPRIV, otherwise
# edit defs.h for an unused internet port address (PORT = xxxx)
# If you want all the cards up, define BARNEYS
#
DEFS = -DROOTPRIV

CFLAGS = -g $(DEFS)

all:	bj dealer

bj:		bj.o sockets.o sockio.o
	cc $(CFLAGS) -o bj bj.o sockets.o sockio.o -lcurses -ltermlib

dealer:		dealer.o opensock.o sockio.o table.o hand.o
	cc $(CFLAGS) -o dealer dealer.o opensock.o sockio.o table.o hand.o

bj.o process.o sockets.o sockio.o:	defs.h

dealer.o opensock.o table.o hand.o:	defs.h

pitboss.o:	defs.h

clean:	
	rm *.o bj dealer

lint:	lintbj lintdealer

lintbj:
	lint $(DEFS) bj.c sockets.c sockio.c

lintdealer:
	lint $(DEFS) dealer.c opensock.c sockio.c table.c hand.c
