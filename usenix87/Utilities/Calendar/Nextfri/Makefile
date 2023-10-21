# $Header$
#
# Makefile: nextfri
#       By: Marc Ries 
#
DESTDIR= 
MANDIR = 
CC     = /bin/cc
OPTS   = -O
DEFS   = -DTEST
CFLAGS = $(OPTS) $(DEFS)
SRCS   = nextfri.c
OBJS   = nextfri.o
LIBS   = 

all:	nextfri

nextfri:	$(OBJS)
	$(CC) $(CFLAGS) -o nextfri $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) a.out core

shar:
	shar -v nextfri.[1clyh] Makefile > nextfri.shr

install:	all
	install -s nextfri $(DESTDIR)
 
man:
	install -c -m 644 nextfri.1 $(MANDIR)
 
nextfri.o:	nextfri.c
