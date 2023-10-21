#    Makefile for ARC
CFLAGS = -g
#  CFLAGS = -O

OBJS = arc.o arcadd.o arccode.o arccvt.o arcdel.o arcdir.o \
arcdos.o arcext.o arcio.o arclst.o arclzw.o arcmatch.o arcpack.o \
arcsq.o arcsvc.o arctst.o arcunp.o arcusq.o arcmisc.o


SRCS = arc.c arcadd.c arccode.c arccvt.c arcdel.c arcdir.c \
arcdos.c arcext.c arcio.c arclst.c arclzw.c arcmatch.c arcpack.c \
arcs.c arcsq.c arcsvc.c arctst.c arcunp.c arcusq.c arcmisc.c

arc:            ${OBJS}
	cc  ${CFLAGS} -o arc ${OBJS}
${OBJS}:        arc.h
arc.h:          arcm.h arcs.h
	touch arc.h
