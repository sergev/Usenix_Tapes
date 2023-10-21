# dbm file to save scores in
SCOREFILE = /usr/games/lib/spacewar

# set to 1 if you want to keep scores in the format user@host.
# set to 0 if you want just the user name.
BYMACHINE = 1

# place to put the manual page
MANLOC = /usr/man/man6/spacewar.6

# if you are running release 3.2, use this .c.o definition
.c.o:
	cc -c $(CFLAGS) $< /usr/lib/f68881.il

# if you are not running release 3.2, use the following .c.o definition
#.c.o:
#	cc -S $(CFLAGS) $<
#	sed -f mathfix $*.s >tmp.s
#	as -20 -o $@ tmp.s
#	rm -f $*.s tmp.s

CFLAGS = -O -f68881 -DSCOREFILE=\"$(SCOREFILE)\" -DBYMACHINE=$(BYMACHINE)

SWOBJS = spacewar.o
SWLIBS= -lm -lsuntool -lsunwindow -lpixrect
SWDOBJS = spaceward.o
SWDLIBS = -lm -lsunwindow -ldbm
DISTFILES = spacewar.6 README Makefile spacewar.h spacewar.icon spacewar.c spaceward.c mathfix

all: spacewar spaceward

spacewar: $(SWOBJS)
	cc $(CFLAGS) -o spacewar $(SWOBJS) $(SWLIBS)

spaceward: $(SWDOBJS)
	cc $(CFLAGS) -o spaceward $(SWDOBJS) $(SWDLIBS)

score: score.c
	cc $(CFLAGS) -o score score.c

install: spacewar spaceward
	install -s -m 4711 -o root spacewar /usr/games
	install -s -o root spaceward /usr/games/lib
	cp spacewar.6 $(MANLOC)

installd: spaceward
	install -s -o root spaceward /usr/games/lib

spacewar.o: spacewar.h

spaceward.o: spacewar.h

kit: $(DISTFILES)
	shar -f swshar -c $(DISTFILES)

clean:
	/bin/rm -f core spacewar spaceward *.o kit *.s
