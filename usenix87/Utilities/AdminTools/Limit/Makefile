# Makefile for limit

all: limit install

limit: limit.o
	cc limit.o -o limit

install: 
	install -m 6755 -c -s limit /usr/local/limit

shar:
	shar -a limit.1 limit.c Makefile >limit.shar
