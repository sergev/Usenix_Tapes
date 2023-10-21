CFLAGS =  -O -DSTRINGS -DBCOPY
SOURCES =  Execute.c Extern.h\
	GetPatFile.c Global.c MakeDesc.c \
	match.h match.c MatchFound.c \
	MkDescVec.c MoveResidue.c PrintLine.c PutUsage.c Search.s
OBJECTS = Execute.o \
	GetPatFile.o Global.o MakeDesc.o \
	match.o MatchFound.o \
	MkDescVec.o MoveResidue.o Search.o PrintLine.o PutUsage.o
BASEFILES = $(SOURCES) Makefile README match.p
match: $(OBJECTS)
	cc -o match $(CFLAGS) $(OBJECTS)
shar:
	shar $(BASEFILES) >match.shar
install: match
	install -c -s match /usr/public/match
man: /usr/man/manp/match.p
/usr/man/manp/match.p: match.p
	rm -f /usr/man/manp/match.p
	cp match.p /usr/man/manp/match.p
	man match > /dev/null
match.o: match.c match.h Extern.h
PutUsage.o: PutUsage.c match.h 
Search.o: Search.s
Execute.o: Execute.c match.h 
MoveResidue.o: MoveResidue.c match.h Extern.h
MatchFound.o: MatchFound.c match.h Extern.h
PrintLine.o: PrintLine.c Extern.h
MkDescVec.o: MkDescVec.c match.h
GetPatFile.o: GetPatFile.c match.h
MakeDesc.o: MakeDesc.c match.h
Global.o: Global.c
listing:
	print -o3 $(BASEFILES) Makefile
#	print -i3 $(BASEFILES) Makefile
clean:
	rm -f *.o match
