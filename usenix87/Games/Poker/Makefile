SRCS=	main.c players.c cards.c util.c showcards.c whowon.c poker.c tellall.c payoff.c betting.c computer.c draw.c scores.c 
OBJS=	main.o players.o cards.o util.o showcards.o whowon.o tellall.o payoff.o betting.o computer.o draw.o scores.o
OBJS2=	poker.o util.o

# define the type of system programs are being made on
# currently this is only important for MASSCOMP machines because some
# socket routine calls and include files are different than 4.2BSD
#CFLAGS= -DMASSCOMP
CFLAGS= -DVAX

pokerd:		$(OBJS)
#	cc $(OBJS) -lsocket -o pokerd
	cc $(OBJS) -o pokerd

poker:		$(OBJS2)
#	cc $(OBJS2) -lcurses -ltermlib -lsocket -o poker 
	cc $(OBJS2) -lcurses -ltermlib -o poker 

listings:
	lpr $(SRCS)

# This 'target' simply makes a new Makefile (after saving the current one
# in Makefile.old) with added dependencies for header files included
# in the listed source files.
depend:
	mv Makefile Makefile.old
	sed -n '1,/^# DO NOT REMOVE THIS LINE/p' Makefile.old > Makefile
	grep '^#[ 	]*include[ 	]*["]' $(SRCS) | \
		sed -e 's/\.[^:]*:#[ 	]*include[ 	]*/.o:	/' \
		    -e 's/"\([^"]*\)".*/\1/' \
		    -e 's@<\([^>]*\)>.*@/usr/include/\1@' >> Makefile

# Lines after this block of comments are replace every time the new dependency
# information is created using 'make depend'.  The next line is crucial.
# DO NOT REMOVE THIS LINE
main.o:	cards.h
main.o:	players.h
main.o:	betting.h
main.o:	draw.h
main.o:	whowon.h
main.o:	payoff.h
main.o:	showcards.h
main.o:	util.h
main.o:	tellall.h
main.o:	limits.h
players.o:	players.h
players.o:	util.h
players.o:	port.h
players.o:	scores.h
cards.o:	cards.h
util.o:	util.h
showcards.o:	showcards.h
showcards.o:	players.h
showcards.o:	cards.h
showcards.o:	util.h
whowon.o:	whowon.h
whowon.o:	players.h
whowon.o:	cards.h
whowon.o:	showcards.h
whowon.o:	tellall.h
poker.o:	util.h
poker.o:	port.h
poker.o:	limits.h
tellall.o:	tellall.h
tellall.o:	util.h
tellall.o:	players.h
payoff.o:	players.h
payoff.o:	tellall.h
betting.o:	betting.h
betting.o:	players.h
betting.o:	computer.h
computer.o:	computer.h
computer.o:	players.h
computer.o:	cards.h
computer.o:	whowon.h
computer.o:	limits.h
draw.o:	draw.h
draw.o:	players.h
draw.o:	showcards.h
draw.o:	tellall.h
scores.o:	scores.h
