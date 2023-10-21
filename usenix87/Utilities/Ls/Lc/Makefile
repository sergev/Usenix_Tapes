DESTDIR =
#CFLAGS	= -DUSG -O
#LIBS	= -lndir
CFLAGS	= -O

lc:	lc.c
	${CC} ${CFLAGS} -o lc lc.c ${LIBS}

install:	lc
	install -s lc $(DESTDIR)/usr/bin/lc

clean:
	rm -f lc a.out core *.o *.bak
