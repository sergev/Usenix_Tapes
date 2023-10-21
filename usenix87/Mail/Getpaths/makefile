# This makefile is appropriate for 4.2bsd.  For SysV or V7,
# replace "scantree" by "oscantree" everywhere.
CFLAGS = -O
# Use the following for Sys V.
# CFLAGS = -O -Dindex=strchr -Drindex=strrchr

getpaths: getpaths.o scantree.o 
	cc $(CFLAGS)  getpaths.o scantree.o  -o getpaths
