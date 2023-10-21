CFLAGS = -O -I..
GOSUB = ../gosub.a

all: gref

fgo: fgo.o $(GOSUB)
	$(CC) $(CFLAGS) $@.o $(GOSUB)  -o $@

gotst: gotst.o $(GOSUB)
	$(CC) $(CFLAGS) $@.o $(GOSUB)  -o $@

gref: gref.o utime.o bdisp.o $(GOSUB)
	$(CC) $(CFLAGS) $@.o utime.o bdisp.o $(GOSUB) -lcurses -ltermcap -o $@

jgref: gref.o utime.o jbdisp.o $(GOSUB)
	$(CC) $(CFLAGS) gref.o utime.o jbdisp.o $(GOSUB) -o $@

grev: grev.o bdisp.o $(GOSUB)
	$(CC) $(CFLAGS) $@.o bdisp.o $(GOSUB) -lcurses -ltermcap -o $@

jgrev: grev.o bdisp.o $(GOSUB)
	$(CC) $(CFLAGS) grev.o jbdisp.o $(GOSUB) -o $@

doc: gref.n 
	pic gref.n | ditroff -ms
	-lpq

tst: tst.o 
	$(CC) $(CFLAGS) tst.o -o $@
