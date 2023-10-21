# @(#)Makefile	1.6
# modified: Tue Sep 30 15:41:23 EET 1986, by kvvax6!pete
#   include Henry Spencers public domain string handling
# modified: Mon Jul  1 08:43:55 GMT+1:00 1985, by KVVAX6!pete
#   actually I didn't modify the makefile, but I should note
#   somewhere that the version of getopt is now the public domain
#   one from the network, not the system V version
# created : Mon Jul  2 13:53:50 CET 1984, by pete @ Olivetti HQ Milan

# for System V:
# CFLAGS = -Dindex=strchr -O $(CONF) -DUNIXERR -I.

SIZET = int
VOIDSTAR = char *
CONST = 
CONF = -DSIZET=$(SIZET) -DVOIDSTAR='$(VOIDSTAR)' -DCONST='$(CONST)'

CFLAGS = -DBSD -O $(CONF) -DBERKERR -I.

OBJS = wp.o 
UTIL = getopt.o strspn.o strtok.o 
UTILSRC = string.h getopt.c strspn.c strtok.c 

wp: $(OBJS) $(UTIL)
	$(CC) $(OBJS) $(UTIL) -o wp

man:
	@nroff -man wp.1

clean:
	rm -f *.o wp

shar:
	@shar -a README wp.1 wp.c $(UTILSRC) Makefile
