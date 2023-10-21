# Define GETLOGIN if you do not have getlogin(3C) call
# Define TMPFILE  if you do not have tmpfile(3C) call
# Define BSD      if you use index(3C) call instead of strchr(3C)
# Define TERMINFO only if your system supports terminfo(4).
CFLAGS= -O 
LINKFLAGS= -n

CC= cc

CFILES= cctag.c cyntag.c etag.c ldtag.c lintag.c
OBJECTS= cctag.o cyntag.o etag.o ldtag.o lintag.o

# Use -lterminfo if you do not have -lcurses
LIBS= -lcurses

ve:	$(OBJECTS)
	$(CC) $(LINKFLAGS) -o ve $(OBJECTS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $*.c
