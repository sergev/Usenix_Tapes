S=# May be set to ``VAX''

string.o : VbyteSM.o string.h string.cc
	ccc -c string.cc
	ld -r string.o VbyteSM.o
	mv a.out string.o

VbyteSM.o : VbyteSM.h VbyteSM.cc
	ccc -c -D$(S) VbyteSM.cc

printVbyteSM :
	lpr -p -w160 -Pi300 VbyteSM.h VbyteSM.cc

printstring :
	lpr -p -w160 -Pi300 string.h string.cc
