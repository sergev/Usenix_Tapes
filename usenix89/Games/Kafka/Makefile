
# RCS Info: $Revision: $ on $Date: $
#           $Source: $
# Copyright (c) 1985 Wayne A. Christopher 
#	Permission is granted to do anything with this code except sell it
#	or remove this message.
#
# The makefile for everything.

CFILES = dstrans.c etc.c funwords.c init.c main.c \
	maketext.c output.c psubr.c stuff.c

KFILES = babble.k flame.k insult.k

HFILES = defs.h graph.h kafgraf.h

OFILES = dstrans.o init.o main.o psubr.o stuff.o \
	lex.yy.o y.tab.o

RUNOFILES = etc.o maketext.o output.o

all: kafka insult babble flame

kafka: $(OFILES)
	cc -o kafka $(OFILES)

insult: insult.k $(RUNOFILES)
	./kafka insult.k
	cc -c kaf.spec.c kaf.text.c
	cc -o insult $(RUNOFILES) kaf.spec.o kaf.text.o

babble: babble.k $(RUNOFILES)
	./kafka babble.k
	cc -c kaf.spec.c kaf.text.c
	cc -o babble $(RUNOFILES) kaf.spec.o kaf.text.o

flame:	flame.k $(RUNOFILES)
	./kafka flame.k
	cc -c kaf.spec.c kaf.text.c
	cc -o flame $(RUNOFILES) kaf.spec.o kaf.text.o

lex.yy.c: lexical.l y.tab.h
	lex lexical.l
	egrep -v '^# define (input|unput)' < lex.yy.c > ltmp
	mv ltmp lex.yy.c

y.tab.c y.tab.h: parse.y
	yacc -d parse.y

