SHELL= /bin/sh
BIN= /usr/lbin
CC= /usr/ghs/compiler/gcc

install: tohex
	install -f $(BIN) tohex

tohex:	tohex.o
	$(CC) -o tohex tohex.o

clean:
	rm -f tohex.o tohex
