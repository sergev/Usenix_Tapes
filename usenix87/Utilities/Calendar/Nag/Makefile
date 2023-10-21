#
# Makefile for the nag/okok system.
#

SHAR1 =		README nag.1 Makefile 
SHAR2 =		$(SRCS)

# -DSYS5 for System V, nothing for BSD.
DEFS =		# -DSYS5
OPTDEB =	-O
CFLAGS =	$(DEFS) $(OPTDEB)
OBJS =		gdate.o nag.o
SRCS =		gdate.c nag.c

all:		nag okok

clean:;		rm -f $(OBJS) nag okok gdate OUT LINT a.out core

nag:		$(OBJS)
		cc -g -o nag $(OBJS)

okok:		nag
		ln nag okok

gdate:		gdate.c
		cc -DDEBUG $(CFLAGS) gdate.c -o gdate

lint:		LINT

LINT:		$(SRCS)
		lint $(DEFS) $(SRCS) > LINT

shar:		nag1.shar nag2.shar

nag1.shar:	$(SHAR1)
		shar -a $(SHAR1) > nag1.shar

nag2.shar:	$(SHAR2)
		shar -a $(SHAR2) > nag2.shar

