# Makefile for program 'p2c' on 4.2 BSD Unix

CFLAGS = -g
SHELL = /bin/csh

p2c: p2c.o proc.o doalloc.o
	cc $(CFLAGS) p2c.o proc.o doalloc.o -o p2c
p2c.lint: p2c.c proc.c doalloc.c
	lint p2c.c proc.c doalloc.c >& p2c.lint
	-more p2c.lint

p2c.o: p2c.h ktypes.h
proc.o: p2c.h ktypes.h
