#*********************************************************************
#
# Title:	btree.fe.M
#
# Function:	btree front-end makefile
#
#*********************************************************************
#
# @(#)btree.fe.M	1.1 7/16/86

btree.fe:	btree.fe.o btree.o btree.prt.h
	cc -O -o btree.fe btree.fe.o

btree.fe.o:	btree.fe.c btree.fe.h
	cc -c -O btree.fe.c

btree.o:	btree.c btree.h
	cc -c -O btree.c
