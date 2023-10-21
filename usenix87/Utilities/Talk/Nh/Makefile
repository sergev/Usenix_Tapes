#	Makefile for Sys V talk
#
talk:	talk.c
	cc -O -o talk talk.c -lcurses
talk.man:	talk.1
	nroff -man talk.1 > talk.man
