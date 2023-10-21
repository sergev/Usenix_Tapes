# if compiling on System V, use -DSYSV
CFLAGS=-O -DSYSV
CFLAGS=-O

postlint: postlint.o
	cc -o postlint postlint.o

archive: README postlint.1 makefile postlint.c human.c
	shar -a README postlint.1 makefile postlint.c human.c > archive
