install=	/etc/ekinstall
BINDIR =	/usr/local/bin
GRAMMARDIR =	/usr/local/lib/number
DEBUG =		-O
CFLAGS =	$(DEBUG) -DGRAMMARDIR=\"$(GRAMMARDIR)/\" \
		-Drandom=lrand48 -Dsrandom=srand48 \
		-Dindex=strchr -Drindex=strrchr
SHELL=	/bin/sh

number:		number.o
	cc $(DEBUG) number.o -o number

install:	install-number install-lang

install-number:	number
	$(install) number $(BINDIR)

install-lang:
	-mkdir $(GRAMMARDIR)
	(cd lang; tar cf - .) | (cd $(GRAMMARDIR); tar xf -)

clean:
	rm -f *.o number *~ core lang/*~
