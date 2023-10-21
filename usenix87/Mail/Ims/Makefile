#! /bin/make -f
###########################################################################
#									  #
#			      Makefile for IMS				  #
#									  #
###########################################################################
#									  #
# Set SYSTEM according to which version of Unix you are running:	  #
#									  #
#	BSD - 4.2BSD							  #
#	USG - AT&T System III, System V; Xenix 3.0, 5.0; Unisoft System V #
#									  #
# V7, 4.1BSD and Unisoft System III are NOT supported at present.  (They  #
# could be added with a little hacking, but I can't test it.)		  #
#									  #
###########################################################################

SYSTEM = USG

###########################################################################
###############################   Make IMS   ##############################
###########################################################################

SHELL = /bin/sh
SHAR = shar
CFLAGS = -O -D$(SYSTEM)
SFLAGS = -v
ISRC = ims.c imsread.c imsdel.c imsinc.c imssend.c imsset.c imsalias.c
GSRC = ndir.c eopen.c
HDRS = ims.h ndir.h
MAN = ims.1
MISC = Makefile
IOBJ = ims.o imsread.o imsdel.o imsinc.o imssend.o imsset.o imsalias.o
GOBJ = ndir.o eopen.o
OBJS = $(IOBJ) $(GOBJ)

##########################################################################

all: ims ims.man

ims: $(OBJS)
	cc -O -o ims $(OBJS)

ims.man: $(MAN)
	nroff -man ims.1 > ims.man

shar:
	$(SHAR) $(SFLAGS) $(ISRC) $(GSRC) $(HDRS) $(MAN) $(MISC) > ims.shar

$(IOBJ): ims.h
