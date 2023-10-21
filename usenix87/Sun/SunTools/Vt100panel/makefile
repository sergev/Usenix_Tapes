CC=cc -g
LIBS=	-lsuntool -lsunwindow -lpixrect
LDFLAGS=
LIBDIR=libdir

vt100tool: vt100tool.c vconfig.h vshelltool.icon
	${CC}   -o vt100tool vt100tool.c ${LIBDIR}/nlibsuntool.a -lsunwindow -lpixrect

vttest: vttest.c
	${CC} ${LDFLAGS} -o vttest vttest.c


