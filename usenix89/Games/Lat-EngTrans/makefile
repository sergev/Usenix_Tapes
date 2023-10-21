#
# makefile for suzy
#

all:		suzy mkit main.o mkdict.o tsort.o dict.o idict.o suf.o

suzy:		suzy.l
		liszt -qrw suzy.l -o suzy

mkit:		mkit.l
		liszt -qrw mkit.l -o mkit

main.o:		main.l
		liszt -qw main.l

mkdict.o:	mkdict.l
		liszt -qw mkdict.l

tsort.o:	tsort.l
		liszt -qw tsort.l

dict.l:		dictionary
		echo dict.l | mkit

dict.o:		dict.l
		liszt -qw dict.l

idict.l:	idictionary.l
		echo idict.l | mkit

idict.o:	idict.o
		liszt -qw idict.l

suf.l:		suffixes.l
		echo suf.l | mkit

suf.o:		suf.l
		liszt -qw suf.l
