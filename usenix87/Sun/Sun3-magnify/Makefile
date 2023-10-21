# Makefile to compile magnifying glass
# by Scott Schwartz, Dec 1986

# flags.  normally just -O
CFLAGS=-O
LIBS= -lsuntool -lsunwindow -lpixrect

# dependencies for glass
glass: glass.o 
	cc $(CFLAGS) -o glass glass.o $(LIBS)

glass.o: glass.c patchlevel.h Makefile
	cc $(CFLAGS) -c glass.c

# utility stuff
clean:
	rm -f glass glass.o
lint:
	lint -hx glass.c $(LIBS)
shar:
	shar README Makefile glass.c glass.icon patchlevel.h >glass.shar
