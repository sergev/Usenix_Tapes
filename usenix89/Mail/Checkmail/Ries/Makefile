#
# Makefile: checkmail
#  Created: January 1986
#       By: Marc Ries
#
CC     = /bin/cc
OPTS   = -O -s 
DEFS   = -DBSD
CFLAGS = $(OPTS) $(DEFS)
SRCS   = getopt.c checkmail.c
OBJS   = getopt.o checkmail.o
LIBS   = 

all:	checkmail

checkmail:	$(OBJS)
	$(CC) $(CFLAGS) -o checkmail $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) a.out core

lint:
	lint -uvhp $(DEFS) $(SRCS)

shar:
	shar -v Makefile checkmail.[1clyh] getopt.c > checkmail.shr

edit:
	vi $(SRCS)
 
checkmail.o:	checkmail.c
