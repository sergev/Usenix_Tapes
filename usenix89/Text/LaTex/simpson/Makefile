DEST=/usr/local/bin
LFLAGS=
YFLAGS=
LDFLAGS=-s
CFLAGS=-O
LOADLIBES=-ltrw -lprofile -lglob -ll

all: index

index: indexlex.c index.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o index index.o $(LOADLIBES)

indexlex.c: indexlex.l

index.o: indexlex.c

install: all
	install index $(DEST)

clean:
	-rm *.o lex.yy.c y.tab.c indexlex.c index
