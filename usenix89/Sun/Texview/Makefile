CFLAGS = -fsingle '-DFONTAREA="/usr/lib/dvifont/"' -O
texview:	texview.o
	cc -o texview ${CFLAGS} texview.o -lsuntool -lsunwindow -lpixrect
texview.o:	texview.c
	cc -c ${CFLAGS} texview.c
