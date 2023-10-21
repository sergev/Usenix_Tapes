# @(#)Makefile	1.1 (TRW) 8/19/85
SOURCES=shar.o sharchild.o getopt.o
DEST=/usr/local/bin
LIB=/usr/local/lib
# Add -lndir for 4.1BSD
LIBS=
# Change to -DBSD41 for 4.1BSD
CFLAGS=-DSHARCHILD=\"$(LIB)/sharchild\" -O -DBSD42

all: $(SOURCES)
	$(CC) $(CFLAGS) -o shar shar.o getopt.o
	$(CC) $(CFLAGS) -o sharchild sharchild.o getopt.o $(LIBS)

install: all
	install -s shar $(DEST)/shar
	install -s sharchild $(LIB)/sharchild

clean:
	rm -f *.o shar sharchild
