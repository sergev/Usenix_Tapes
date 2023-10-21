#
# RCS Info: $Revision: 1.4 $ on $Date: 86/04/02 10:33:03 $
#           $Source: /ic4/faustus/src/spellfix/RCS/Makefile,v $
# Copyright (c) 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
#
# Makefile for the spellfix system
#
# For dependencies, see below in LDFLAGS and CFLAGS.

CC = cc

HFILES	=\
	spellfix.h

CFILES =\
	adjacent.c\
	baseword.c\
	compare.c\
	fixwords.c\
	freqcount.c\
	getbuf.c\
	makeadj.c\
	makemod.c\
	readdict.c\
	spellcheck.c\
	spellfix.c

SFILES =

OFILES =\
	adjacent.o\
	baseword.o\
	compare.o\
	fixwords.o\
	freqcount.o\
	getbuf.o\
	makeadj.o\
	makemod.o\
	readdict.o\
	spellcheck.o\
	spellfix.o

SOURCE=	$(CFILES) $(SFILES)
ALLFILES= $(SOURCE) $(HFILES)

INCLUDE = -I../h

# Compile flags are as follows:
#	UNIX		Compiling under unix
#	BSD		Compiling under 4.2 BSD unix
#	VAX		Compiling for a vax

DEFINES = -DUNIX -DBSD -DVAX

CFLAGS = $(DEFINES) $(INCLUDE) -g
LINTFLAGS = -huca -DLINT $(DEFINES) $(INCLUDE)
LDFLAGS = -g -z -lcurses -ltermlib -lm

.c.o: $*.c
	$(CC) $(CFLAGS) -c $*.c

all: baseword spellcheck spellfix makeadj freqcount wordchange

baseword: 	baseword.o
	$(CC) baseword.o $(LDFLAGS) -o baseword

spellcheck:	spellcheck.o readdict.o
	$(CC) spellcheck.o readdict.o $(LDFLAGS) -o spellcheck

spellfix:	spellfix.o readdict.o compare.o adjacent.o fixwords.o \
		makemod.o getbuf.o
	$(CC) spellfix.o readdict.o compare.o adjacent.o fixwords.o \
			makemod.o getbuf.o $(LDFLAGS) -o spellfix

makeadj:	makeadj.o
	$(CC) makeadj.o $(LDFLAGS) -o makeadj

wordchange:	wordchange.o
	$(CC) wordchange.o $(LDFLAGS) -o wordchange

freqcount:	freqcount.o
	$(CC) freqcount.o $(LDFLAGS) -o freqcount

lint:	$(SOURCE)
	lint $(LINTFLAGS) $(SOURCE)

qgrind: $(ALLFILES)
	qgrind -lc $(ALLFILES)

source:	$(SOURCE) 

tags: 	/tmp
	ctags -t ../*/*.c ../*/*.h > /dev/null 2>&1

wc: $(ALLFILES)
	@wc $(ALLFILES)

print: $(ALLFILES)
	@pr $(ALLFILES)

clean:
	rm -f $(OFILES) foo make.out tags a.out

$(ALLFILES):
	co -l $@

depend: $(SOURCE)
	cc -M $(CFLAGS) $(CFILES) $(SFILES) > makedep
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	ed - Makefile < eddep
	rm eddep makedep 
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

#-----------------------------------------------------------------
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
adjacent.o: adjacent.c
adjacent.o: ./spellfix.h
adjacent.o: /usr/include/stdio.h
adjacent.o: /usr/include/ctype.h
baseword.o: baseword.c
baseword.o: ./spellfix.h
baseword.o: /usr/include/stdio.h
baseword.o: /usr/include/ctype.h
compare.o: compare.c
compare.o: ./spellfix.h
compare.o: /usr/include/stdio.h
compare.o: /usr/include/ctype.h
fixwords.o: fixwords.c
fixwords.o: ./spellfix.h
fixwords.o: /usr/include/stdio.h
fixwords.o: /usr/include/ctype.h
fixwords.o: /usr/include/curses.h
fixwords.o: /usr/include/stdio.h
fixwords.o: /usr/include/sgtty.h
fixwords.o: /usr/include/sys/ioctl.h
fixwords.o: /usr/include/sys/ttychars.h
fixwords.o: /usr/include/sys/ttydev.h
fixwords.o: /usr/include/sys/time.h
fixwords.o: /usr/include/time.h
freqcount.o: freqcount.c
freqcount.o: /usr/include/stdio.h
freqcount.o: /usr/include/ctype.h
getbuf.o: getbuf.c
getbuf.o: /usr/include/sys/ioctl.h
getbuf.o: /usr/include/sys/ttychars.h
getbuf.o: /usr/include/sys/ttydev.h
getbuf.o: /usr/include/ctype.h
getbuf.o: /usr/include/curses.h
getbuf.o: /usr/include/stdio.h
getbuf.o: /usr/include/sgtty.h
makeadj.o: makeadj.c
makeadj.o: /usr/include/stdio.h
makeadj.o: /usr/include/ctype.h
makemod.o: makemod.c
makemod.o: ./spellfix.h
makemod.o: /usr/include/stdio.h
makemod.o: /usr/include/ctype.h
readdict.o: readdict.c
readdict.o: ./spellfix.h
readdict.o: /usr/include/stdio.h
readdict.o: /usr/include/ctype.h
spellcheck.o: spellcheck.c
spellcheck.o: ./spellfix.h
spellcheck.o: /usr/include/stdio.h
spellcheck.o: /usr/include/ctype.h
spellfix.o: spellfix.c
spellfix.o: ./spellfix.h
spellfix.o: /usr/include/stdio.h
spellfix.o: /usr/include/ctype.h
spellfix.o: /usr/include/sys/file.h
spellfix.o: /usr/include/curses.h
spellfix.o: /usr/include/stdio.h
spellfix.o: /usr/include/sgtty.h
spellfix.o: /usr/include/sys/ioctl.h
spellfix.o: /usr/include/sys/ttychars.h
spellfix.o: /usr/include/sys/ttydev.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
