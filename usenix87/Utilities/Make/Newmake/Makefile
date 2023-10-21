#
# Makefile: newmake
#  Created: Wednesday, December 11, 1985
#       By: Marc A. Ries (FCOMP, Anaheim Hills, CA 92807-2321)
#
CC = /bin/cc
CFLAGS = -O -s -DDAYTIME 
SRCS = newmake.c
OBJS = newmake.o

all:	newmake

newmake:	$(OBJS)
	$(CC) $(CFLAGS) -o newmake $(OBJS)

clean:
	rm -f $(OBJS) a.out core

shar:
	shar -v -p Z ReadMe newmake.[clyh] Makefile > newmake.shr

edit:
	vi $(SRCS)
 
newmake.o:	newmake.c
