#
# $Header: Makefile,v 1.1 85/09/19 13:24:27 mcooper Exp $
#
# $Log:	Makefile,v $
# Revision 1.1  85/09/19  13:24:27  mcooper
# Initial revision
# 
#
#
# Makefile for rcsit.c
#
BIN = /usr/unsupp
MAN = /usr/man/manu/man1

rcsit: rcsit.c
	cc -O -s -o rcsit rcsit.c

dbx: rcsit.c
	cc -g rcsit.c

debug: rcsit.c
	cc -g -DDEBUG rcsit.c

man: rcsit.1
	nroff -man rcsit.1 > rcsit.man

install: rcsit rcsit.1
	mv rcsit $(BIN)
	cp rcsit.1 $(MAN)
