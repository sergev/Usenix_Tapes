#
# Makefile to use maping software.
#
PROGS=	ndmap map
MAP=	../map/u.*

all:	ndmap uucp usenet

ndmap:	ndmap.c
	cc -O ndmap.c -lplot -lm -o ndmap

map:	map.c
	cc -O map.c -o map

uuedge:
	pathalias -l ucbvax -g uuedge $(MAP) > /dev/null

uucp:	uuedge map
	map uuedge $(MAP) | sort -n -u -o uucp

usedge:
	sed '/^#U[^,]*,/s/,/ /g' $(MAP) | awk -f usenet.awk > usedge

usenet:	usedge map
	map usedge $(MAP) | sort -n -u -o usenet

clean:
	rm -f ndmap map edge uucp usenet uuedge usedge
