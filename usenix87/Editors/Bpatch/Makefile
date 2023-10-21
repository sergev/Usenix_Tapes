# This may have to change on your system.
# You may have to fiddle with various combinations of curses, termcap,
# terminfo, etc.  Good luck!
LIBS	= -lcurses -ltermcap
#LIBS	= -ltermlib
#LIBS	= -lterminfo

# However you need getopt.
GETOPT	= -lgetopt

# If you're running on a 4.[23] system, you probably want this.
WORK	= -DMOD_HAX

CFLAGS	= -O $(WORK)

bpatch:		bpatch.o
	$(CC) -o bpatch $(CFLAGS) bpatch.o $(GETOPT) $(LIBS)

install:
	@echo Use cp to put things where you want them
