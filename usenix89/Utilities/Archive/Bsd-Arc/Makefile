#
#	Makefile for Hack-attack 1.3
#	VAX 11/780 BSD4.2 "ARC" utility
#
CFLAGS = -O

OBJS = arc.o arcadd.o arccode.o arccvt.o arcdel.o arcdir.o \
arcdos.o arcext.o arcio.o arclst.o arclzw.o arcmatch.o arcpack.o arcrun.o \
arcsq.o arcsvc.o arctst.o arcunp.o arcusq.o arcvax.o arcmisc.o


SRCS = arc.c arcadd.c arccode.c arccvt.c arcdel.c arcdir.c \
arcdos.c arcext.c arcio.c arclst.c arclzw.c arcmatch.c arcpack.c arcrun.c \
arcs.c arcsq.c arcsvc.c arctst.c arcunp.c arcusq.c arcvax.c arcmisc.c

arc:		${OBJS}
	cc  -O -o arc ${OBJS}
arc.o:		arc.h
	cc -O -c arc.c
arcadd.o:	arc.h
	cc -O -c arcadd.c
arccode.o:	arc.h
	cc -O -c arccode.c
arccvt.o:	arc.h
	cc -O -c arccvt.c
arcdel.o:	arc.h
	cc -O -c arcdel.c
arcdir.o:	arc.h
	cc -O -c arcdir.c
arcdos.o:	arc.h
	cc -O -c arcdos.c
arcext.o:	arc.h
	cc -O -c arcext.c
arcio.o:	arc.h
	cc -O -c arcio.c
arclst.o:	arc.h
	cc -O -c arclst.c
arclzw.o:	arc.h
	cc -O -c arclzw.c
arcmatch.o:	arc.h
	cc -O -c arcmatch.c
arcmisc.o:	arc.h
	cc -O -c arcmisc.c
arcpack.o:	arc.h
	cc -O -c arcpack.c
arcrun.o:	arc.h
	cc -O -c arcrun.c
arcs.o:		arcm.h
	cc -O -c arcs.c
arcsq.o:	arc.h
	cc -O -c arcsq.c
arcsvc.o:	arc.h
	cc -O -c arcsvc.c
arctst.o:	arc.h
	cc -O -c arctst.c
arcunp.o:	arc.h
	cc -O -c arcunp.c
arcusq.o:	arc.h
	cc -O -c arcusq.c
arcvax.o:	arc.h
	cc -O -c arcvax.c
arc.h:		arcm.h arcs.h
	touch arc.h
