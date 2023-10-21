CFLAGS=-O

GOPT=
# BSD people remove the following comment
# GOPT=getopt.o
markov3: markov3.o $(GOPT)
	cc $(CFLAGS) markov3.o $(GOPT) -o markov3

markov3.c:	markov3.l
		lex markov3.l
		mv lex.yy.c markov3.c

shar:
		shar README markov3.l markov3.6 Makefile getopt.c PATCHLEVEL > shar
