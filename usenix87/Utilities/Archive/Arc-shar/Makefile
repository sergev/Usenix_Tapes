# Makefile for arc and xarc.  Doesn't do marc at present.
# common src
SRC=arcadd.c arccode.c arccvt.c arcdel.c arcio.c arclst.c arclzw.c arcmatch.c arcpack.c arcrun.c arcsq.c arcsvc.c arctst.c arcunp.c arcusq.c

# common obj
OBJ=arcadd.o arccode.o arccvt.o arcdel.o arcio.o arclst.o arclzw.o arcmatch.o arcpack.o arcrun.o arcsq.o arcsvc.o arctst.o arcunp.o arcusq.o

# for unix
USRC=arcunix.c tm_to_time.c
UOBJ=arcunix.o tm_to_time.o

# for dos
DSRC=arcdir.c arcdos.c
DOBJ=arcdir.o arcdos.o

# choose one pair
SRCS=$(SRC) $(USRC)
OBJS=$(OBJ) $(UOBJ)
# SRCS=$(SRC) $(DSRC)
# OBJS=$(OBJ) $(DOBJ)

HEADERS=arc.h arcs.h

AOBJ=$(OBJS) arc.o aarcext.o
XOBJ=$(OBJS) xarc.o xarcext.o

CFLAGS=-DSYS5 -O

SHAR=shar -c -p X

arc:	$(AOBJ)
	cc -o arc $(AOBJ)

aarcext.o:	arcext.c
	cc $(CFLAGS) -c -DARC arcext.c
	mv arcext.o aarcext.o

xarc:	$(XOBJ)
	cc -o xarc $(XOBJ)

xarcext.o:	arcext.c
	cc $(CFLAGS) -c -DXARC arcext.c
	mv arcext.o xarcext.o

shar:
	cat README > arc.shar.01
	$(SHAR) Makefile arc.c arc.h arcadd.c arccode.c arccvt.c arcdel.c arcext.c arcio.c > arc.shar.02
	$(SHAR) arclst.c arclzw.c arcmatch.c arcpack.c arcrun.c arcs.h > arc.shar.03
	$(SHAR) arcsq.c arcsvc.c arctst.c arcunix.c arcunp.c arcusq.c marc.c tm_to_time.c xarc.c > arc.shar.04
