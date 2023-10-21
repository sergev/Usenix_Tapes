#
#  help Makefile, 1.1	7/27/84
#
CFLAGS= -O
DESTDIR= /usr/local/bin

help:	help.o 
	${CC} ${CFLAGS} help.o -o help

install: help
	install -s help $(DESTDIR)

install_man:
	cp help.1 /usr/man/manl/help.l

help.o: help.h /usr/include/sys/file.h /usr/include/sys/param.h

help.shar: README Makefile help.1 help.c help.h
	shar -vc README Makefile help.1 help.c help.h > help.shar

clean:  
	rm -f a.out core help.o help
