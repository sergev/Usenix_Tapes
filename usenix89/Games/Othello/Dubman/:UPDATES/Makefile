DFLAGS = -O

CFLAGS = $(DFLAGS)

otool:	otool.o othello.o
	cc $(CFLAGS) -o otool otool.o othello.o -lsuntool -lsunwindow -lpixrect

othello: othello.o
	cc $(CFLAGS) -o othello othello.o

othello.o: othello.c
	cc $(CFLAGS) -DOTHELLOTOOL -c othello.c
