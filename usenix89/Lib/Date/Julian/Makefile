#
# Makefile: day
#  Created: Thursday, September 04, 1986
#       By: Marc Ries
#
CC     = /bin/cc
OPTS   = -O 
DEFS   = 
CFLAGS = $(OPTS) $(DEFS)
SRCS   = day.c jday.c jdate.c
OBJS   = jday.o jdate.o
LIBS   = 
RANLIB = ucb ranlib libjdates.a

all:	day libjdates

day:	day.o  
	$(CC) $(CFLAGS) -o day day.c $(LIBS)

libjdates:	jday.o jdate.o  
	ar r libjdates.a $(OBJS)
	$(RANLIB)	

clean:
	rm -f $(OBJS) a.out core

shar:
	shar -v day.[1clyh] Makefile > day.shr

edit:
	vi $(SRCS)
 
day.o:	day.c
