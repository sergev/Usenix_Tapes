# makefile for 'which'
# vix 25jul86 [written]

CFLAGS		=	-O
DESTDIR		=	/mnt/local/bin

.c.o		:
			cc $(CFLAGS) -c $<

all		:	which

which		:	which.o
			cc -o which which.o

which.o		:	which.c

install		:	all
			mv which $(DESTDIR)
			chmod 775 $(DESTDIR)/which

clean		:
			rm -f which.o

kit		:	clean
			shar -bcsv README Makefile which.c > which.shar
