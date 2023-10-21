DEBUG	=
CFLAGS	= -O $(DEBUG)
AFLAGS	= -A-m68010
OFILES	= showpix.o bitmap.o
CC	= /bin/cc
LIBS 	= -lsuntool -lsunwindow -lpixrect

all:	showpix moveicon showicon trimicon
showpix			: $(OFILES)
			$(CC) $(CFLAGS) -o showpix $(OFILES) $(LIBS)

bitmap.o		: bitmap.c bitmap.h
			$(CC) $(CFLAGS) -c bitmap.c

showpix.o		: showpix.c bitmap.h
			$(CC) $(CFLAGS) -c showpix.c

moveicon		: moveicon.c
			$(CC) $(CFLAGS) -o $@ moveicon.c

showicon		: showicon.c
			$(CC) $(CFLAGS) -o $@ showicon.c

trimicon		: trimicon.c
			$(CC) $(CFLAGS) -o $@ trimicon.c
