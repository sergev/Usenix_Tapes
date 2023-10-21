FILES	= receive.c transmit.c addcr.c delcr.c add.c del.c\
          lf2cr.c cr.c lf.c cr2lf.c
OBJS	= receive.o transmit.o addcr.o delcr.o add.o del.o\
          lf2cr.o cr.o lf.o cr2lf.o
LIBS 	= -lcurses -ltermlib
FLAGS	= -O -c -I/sys/h
PATH	= /usr/local
all:		receive transmit
clean:
		rm -f $(OBJS) receive transmit
pr:
		lpr -Plp $(FILES)
install:	receive transmit
		strip receive
		cp receive $(PATH)/xmodrec
		strip transmit
		cp transmit $(PATH)/xmodtran
receive.o:	receive.c
		cc $(FLAGS) receive.c
receive:	receive.o del.o lf.o
		cc -O -o receive del.o lf.o receive.o $(LIBS)
transmit.o:	transmit.c
		cc $(FLAGS) transmit.c
transmit:	transmit.o add.o cr.o
		cc -O -o transmit add.o cr.o transmit.o $(LIBS)
addcr.o:	addcr.c
		cc $(FLAGS) addcr.c
addcr:		addcr.o add.o
		cc -O -o addcr add.o addcr.o
		strip addcr
		cp addcr $(PATH)
delcr.o:	delcr.c
		cc $(FLAGS) delcr.c
delcr:		delcr.o del.o
		cc -O -o delcr del.o delcr.o
		strip delcr
		cp delcr $(PATH)
add.o:		add.c
		cc $(FLAGS) add.c
del.o:		del.c
		cc $(FLAGS) del.c
cr.o:		cr.c
		cc $(FLAGS) cr.c
lf2cr.o:	lf2cr.c
		cc $(FLAGS) lf2cr.c
lf2cr:		lf2cr.o cr.o
		cc -O -o lf2cr lf2cr.o cr.o
		strip lf2cr
		cp lf2cr $(PATH)
lf.o:		lf.c
		cc $(FLAGS) lf.c
cr2lf.o:	cr2lf.c
		cc $(FLAGS) cr2lf.c
cr2lf:		cr2lf.o lf.o
		cc -O -o cr2lf cr2lf.o lf.o
		strip cr2lf
		cp cr2lf $(PATH)
