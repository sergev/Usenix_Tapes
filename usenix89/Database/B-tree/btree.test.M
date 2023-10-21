#*********************************************************************
#
# Title:	btree.test.M
#
# Function:	btree test makefile
#
#*********************************************************************
#
# @(#)btree.test.M	1.1 7/16/86

btree.test:	btree.test.o btree.o btree.prt.h
	cc -O -o btree.test btree.test.o

btree.test.o:	btree.test.c btree.test.h
	cc -c -O btree.test.c

btree.o:	btree.c btree.h
	cc -c -O btree.c
