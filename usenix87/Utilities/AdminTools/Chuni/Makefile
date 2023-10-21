#	Makefile for chuni
PROGNAME = chuni

# programs
CC = cc

# flags
CFLAGS = -O
LDFLAGS =

# directories
BIN = /usr/local/bin

#files
SRCS = chuni.c

all:	$(PROGNAME)

$(PROGNAME):	$(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROGNAME) $(SRCS)

install:	$(PROGNAME)
	ucb install -o root -m u+srwx,og+rx $(PROGNAME) $(BIN)

clean:
	rm -f core *.o $(PROGNAME)

delta:
	sccs delta `sccs tell`

print:
	@print Makefile $(SRCS)
