# make this getopt.o if on a BSD or other non system V
GETOPT=

CFLAGS=-O

all:	traverse shar

shar:	shar.o traverse.o $(GETOPT)
	$(CC) -o shar shar.o traverse.o $(GETOPT)

traverse:
	$(CC) $(CFLAGS) -o traverse -DSTANDALONE traverse.c

install:	all
	/etc/install -f /usr/bin shar
	/etc/install -f /usr/bin traverse
	strip /usr/bin/shar /usr/bin/traverse
	cp shar.1 /usr/man/u_man/man1/shar.1
	cp traverse.3 /usr/man/p_man/man3/traverse.3

clean:
	rm -f shar traverse *.o

shar.shar: Makefile shar.c traverse.c shar.1 traverse.3
	shar -a -d EOF_SHAR Makefile shar.c traverse.c shar.1 traverse.3 > shar.shar
