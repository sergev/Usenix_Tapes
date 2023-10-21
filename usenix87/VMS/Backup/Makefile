#
#
REMOTE=-DREMOTE			# -DREMOTE  use remote tape
SWAP=				# -DSWAP swap bytes
CFLAGS= $(SWAP) $(REMOTE)
LFLAGS=
LIBS= -lrmt   			# remote magtape library
OWNER=tar			# user for remote tape access
MODE=4755
BINDIR=/usr/local/bin
MANSEC=l
MANDIR=/usr/man/man$(MANSEC)

#
vmsbackup: vmsbackup.o getopt.o match.o
	cc $(LFLAGS) -o vmsbackup vmsbackup.o match.o getopt.o $(LIBS)
install:
	install -m $(MODE) -o $(OWNER) -s vmsbackup $(BINDIR)	
	cp vmsbackup.1 $(MANDIR)/vmsbackup.$(MANSEC)
clean:
	rm -f vmsbackup *.o core
shar:
	shar -a README vmsbackup.1 Makefile vmsbackup.c match.c \
	    > vmsbackup.shar
