.SUFFIXES:     .c

LIBS	      = -ltermcap

LINKER	      = cc
CC	      = cc
AR	      = ar
NM            = nm

MAKEFILE      = makefile

SRCS	      = $.c

OBJS	      = $.o

PROGRAMS      = appts\
		autocall\
		eappts\
		callme\
		vcal\
		lcal


all:		$(PROGRAMS)

appts:	appts.c
	$(CC) -O -o appts appts.c

autocall:	autocall.c
	$(CC) -O -o autocall autocall.c

callme:	callme.c
	$(CC) -O -o callme callme.c

eappts:	eappts.c
	$(CC) -O -o eappts eappts.c

vcal:	vcal.c
	$(CC) -O -o vcal vcal.c  $(LIBS)

lcal:	lcal.c
	$(CC) -O -o lcal lcal.c

