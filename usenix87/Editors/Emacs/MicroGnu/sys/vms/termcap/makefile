#
#  FILE
#
#	Makefile    build termcap library
#
#  KEY WORDS
#
#	libraries
#	test functions
#
#  SYNOPSIS
#
#	make		compile the library sources
#	make tests	compile sources for tests
#
#  DESCRIPTION
#
#	Standard make file for building the termcap library and tests.
#
#  AUTHOR
#
#	Fred Fish
#

CC =		cc

TSOURCES =	testtcp.c 
LSOURCES =	tgetent.c tgetflag.c tgetnum.c tgetstr.c tgoto.c tputs.c


TOBJECTS =	testtcp.o
LOBJECTS =	tgetent.o tgetflag.o tgetnum.o tgetstr.o tgoto.o tputs.o

all :		library tests

library :	$(LOBJECTS)

tgetent.o :	tgetent.c
		$(CC) -c $(CFLAGS) tgetent.c

tgetflag.o :	tgetflag.c
		$(CC) -c $(CFLAGS) tgetflag.c

tgetnum.o :	tgetnum.c
		$(CC) -c $(CFLAGS) tgetnum.c

tgetstr.o :	tgetstr.c
		$(CC) -c $(CFLAGS) tgetstr.c

tgoto.o :	tgoto.c
		$(CC) -c $(CFLAGS) tgoto.c

tputs.o :	tputs.c
		$(CC) -c $(CFLAGS) tputs.c

tests :		testtcp

testtcp :	testtcp.o
		$(CC) -o testtcp testtcp.o -ltermcap

testtcp.o :	testtcp.c
		$(CC) -c $(CFLAGS) testtcp.c

#
#	Clean up the directory.
#

clean:
		rm -f *.o *.BAK *.bak nohup.out *.CKP *.tmp 
