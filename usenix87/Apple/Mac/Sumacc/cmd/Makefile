DP=/usr/stanford/bin
DI=/usr/include
PRGS=fromhex kermit macget macput macsbugmap rmaker tohex unlisa rmakerx
SHELLS=macdbmap
CFLAGS= -O -I../h -I${DI} 

all:	$(PRGS)

fromhex:	fromhex.o
	cc -o fromhex fromhex.o

kermit:	kermit.o
	cc -o kermit kermit.o

macget:	macget.o
	cc -o macget macget.o

macput:	macput.o
	cc -o macput macput.o

macsbugmap:	macsbugmap.o
	cc -o macsbugmap macsbugmap.o

rmaker:	rmaker.o
	cc -o rmaker rmaker.o

tohex:	tohex.o
	cc -o tohex tohex.o

unlisa:	unlisa.o
	cc -o unlisa unlisa.o

rmakerx:	rmakerx.o
	cc -o rmakerx rmakerx.o

clean:
	rm -f *.o $(PRGS)

install: 
	cp $(PRGS) $(DP)
	cp $(SHELLS) $(DP)
