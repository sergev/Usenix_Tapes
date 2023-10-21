OBJS = cmstape.o ebcdic.o
CFLAGS = -O
cmstape:	$(OBJS)
	$(CC) -o cmstape $(OBJS)
cmstape.o:	cmstape.c
ebcdic.o:	ebcdic.c

install:	cmstape
	install -s cmstape /usr/local
	cp cmstape.1 /usr/man/manl/cmstape.l
clean:
	rm -f *.o

