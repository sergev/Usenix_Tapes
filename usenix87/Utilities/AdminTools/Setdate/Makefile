#
#  FILE
#
#	Makefile
#
#  SCCS
#
#	@(#)Makefile	1.2	2/10/85
#
#  DESCRIPTION
#
#	Makefile for the system boot utility "setdate" which
#	sets the current date and time.  Accepts a wide variety
#	of date formats.
#
#  AUTHOR
#
#	Fred Fish
#	(Currently at UniSoft Systems Inc.)
#

.l~.c:
	$(GET) $(GFLAGS) -p $< >$*.l
	$(LEX) $(LFLAGS) $*.l
	mv lex.yy.c $*.c
	rm -f $*.l

setdate :	setdate.o
		$(CC) -o $@ $? -ly

setdate.o :	setdate.c lex.c

#
#	Clean up the directory.
#

clean :
		rm -f *.BAK *.o

clobber :
		rm -f setdate.c lex.c setdate


