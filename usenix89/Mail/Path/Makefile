# Makefile for PATH - requires PATHALIAS and the Usenet MAP.
# 
# Should work on any Unix system, but I've only tried it on BSD4.2.
# 
# v1.0	03/25/87	Bruce Holloway (drivax!holloway)

pathalias = /usr/local/bin/pathalias
getopt = /usr/local/bin/getopt.o

# MAP describes the location of the Usenet MAP files.

MAP = *.map /scratch/map/u.* /scratch/map/d.*

all:	pathfix walkdb path

path:	pathmap.a path.o
	cc -o path -O path.o pathmap.a $(getopt)

pathmap.a: $(MAP)
	$(pathalias) $(MAP) >path.out
	pathfix <path.out >path.fix
	walkdb <path.fix
	cc -c *.map.c
	rm pathmap.a
	ar r pathmap.a master.map.o br*.map.o
	ranlib pathmap.a
	rm *.map.*

pathfix: pathfix.o
	cc -o pathfix -O pathfix.o

walkdb:	walkdb.o
	cc -o walkdb -O walkdb.o

path.o walkdb.o: walkdb.h
