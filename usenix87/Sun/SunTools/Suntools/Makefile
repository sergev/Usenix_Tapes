DESTDIR=/usr/local/bin
CFLAGS=	-O
LDFLAGS= -O
LIBS=	-lsuntool -lsunwindow -lpixrect

suntools: suntools.o
	cc ${LDFLAGS} -o suntools suntools.o ${LIBS}
	chmod 4755 suntools
	mv suntools $(DESTDIR)

shar:
	suntools.c README Makefile

suntools.o: /usr/include/suntool/tool_hs.h /usr/include/sys/ioctl.h
suntools.o: /usr/include/sys/dir.h /usr/include/sys/file.h
suntools.o: /usr/include/sys/wait.h /usr/include/sys/resource.h
suntools.o: /usr/include/sys/stat.h /usr/include/errno.h /usr/include/stdio.h
suntools.o: /usr/include/pwd.h /usr/include/suntool/menu.h
suntools.o: /usr/include/suntool/wmgr.h
