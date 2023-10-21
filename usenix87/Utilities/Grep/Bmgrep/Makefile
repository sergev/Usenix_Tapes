CCFLAGS =  -O -Dindex=strchr
SOURCES =  bm.h bm.c Execute.c Extern.h\
	GetPatFile.c Global.c MakeDesc.c MakeSkip.c \
	MatchFound.c \
	MkDescVec.c MoveResidue.c PrintLine.c PutUsage.c Search.c
OBJECTS = bm.o Execute.o \
	GetPatFile.o Global.o MakeDesc.o MakeSkip.o \
	MatchFound.o \
	MkDescVec.o MoveResidue.o Search.o PrintLine.o PutUsage.o
BASEFILES = $(SOURCES) Makefile README bm.1
bm: $(OBJECTS)
	cc -s -o bm $(CCFLAGS) $(OBJECTS)
install: bm
	rm -f /usr/bin/bm
	cp bm /usr/bin/bm
	chmod ugo-w /usr/bin/bm
#	rm /usr/src/public/bm/*
#	cp $(BASEFILES) /usr/src/public/bm
shar:
	/usr/local/bin/shar $(BASEFILES) >bm.shar
man: /usr/man/man1/bm.1
/usr/man/man1/bm.1: bm.1
	rm -f /usr/man/man1/bm.1
	cp bm.1 /usr/man/man1/bm.1
	man bm > /dev/null
bm.o: bm.c bm.h Extern.h
	cc -c $(CCFLAGS) bm.c
PutUsage.o: PutUsage.c bm.h 
	cc -c $(CCFLAGS) PutUsage.c
MakeSkip.o: MakeSkip.c bm.h 
	cc -c $(CCFLAGS) MakeSkip.c
Search.o: Search.c bm.h Extern.h
	cc -c $(CCFLAGS) Search.c
Execute.o: Execute.c bm.h 
	cc -c $(CCFLAGS) Execute.c
MoveResidue.o: MoveResidue.c bm.h Extern.h
	cc -c $(CCFLAGS) MoveResidue.c
MatchFound.o: MatchFound.c bm.h Extern.h
	cc -c $(CCFLAGS) MatchFound.c
PrintLine.o: PrintLine.c Extern.h
	cc -c $(CCFLAGS) PrintLine.c
MkDescVec.o: MkDescVec.c bm.h
	cc -c $(CCFLAGS) MkDescVec.c
GetPatFile.o: GetPatFile.c bm.h
	cc -c $(CCFLAGS) GetPatFile.c
MakeDesc.o: MakeDesc.c bm.h
	cc -c $(CCFLAGS) MakeDesc.c
Global.o: Global.c
	cc -c $(CCFLAGS) Global.c
listing:
# use -o for Sys V, -i for 4.2BSD
#	print -i3 $(BASEFILES)
	print -o3 $(BASEFILES)
clean:
	rm -f *.o a.out foo bar blat junk core
