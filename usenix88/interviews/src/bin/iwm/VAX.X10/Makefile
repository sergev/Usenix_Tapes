#
# InterViews window manager
#

I = ../../../..
T = ../../..
M = VAX.X10
X = X10

AOUT = a.out
DEST = ${I}/bin/${M}/iwm

CC = CC
CFLAGS = -g -I.. -D${X}
LIBS = ${T}/lib/${M}/libInterViews.a
EXTLIBS = -l${X} -lm

LD = CC
LDFLAGS =

OBJS = iwm.o desktop.o dispatch.o lock.o logo.o rubber.o

${AOUT}: ${OBJS} ${LIBS}
	rm -f ${AOUT}
	${LD} ${LDFLAGS} ${OBJS} ${LIBS} ${EXTLIBS} -o ${AOUT}

install: ${AOUT}
	install -s ${AOUT} ${DEST}

iwm.o: ../iwm.c
	${CC} ${CFLAGS} -c ../iwm.c

desktop.o: ../desktop.c
	${CC} ${CFLAGS} -c ../desktop.c

dispatch.o: ../dispatch.c
	${CC} ${CFLAGS} -c ../dispatch.c

lock.o: ../lock.c
	${CC} ${CFLAGS} -c ../lock.c

logo.o: ../logo.c
	${CC} ${CFLAGS} -c ../logo.c

rubber.o: ../rubber.c
	${CC} ${CFLAGS} -c ../rubber.c

clean:
	rm -f *.o a.out core make.out

depend:
	mkdepend ${CFLAGS} -I/usr/include/CC ${OBJS}
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
desktop.o dispatch.o iwm.o: ../InterViews/X10/Xdefs.h
desktop.o dispatch.o: ../InterViews/X10/Xinput.h
desktop.o dispatch.o iwm.o: ../InterViews/Xdefs.h
desktop.o dispatch.o: ../InterViews/Xinput.h
desktop.o logo.o: ../InterViews/bitmap.h
lock.o: ../InterViews/box.h
desktop.o: ../InterViews/canvas.h
desktop.o dispatch.o iwm.o lock.o: ../InterViews/cursor.h
desktop.o dispatch.o iwm.o lock.o logo.o: ../InterViews/defs.h
rubber.o: ../InterViews/defs.h
desktop.o dispatch.o iwm.o lock.o: ../InterViews/frame.h
lock.o: ../InterViews/glue.h
desktop.o dispatch.o iwm.o lock.o: ../InterViews/interactor.h
logo.o: ../InterViews/interactor.h
desktop.o dispatch.o iwm.o: ../InterViews/menu.h
desktop.o dispatch.o iwm.o lock.o logo.o: ../InterViews/paint.h
desktop.o dispatch.o iwm.o lock.o logo.o: ../InterViews/painter.h
desktop.o: ../InterViews/reqerr.h
desktop.o dispatch.o iwm.o lock.o logo.o: ../InterViews/resource.h
rubber.o: ../InterViews/resource.h
desktop.o rubber.o: ../InterViews/rubband.h
desktop.o rubber.o: ../InterViews/rubrect.h
desktop.o dispatch.o iwm.o lock.o: ../InterViews/scene.h
desktop.o dispatch.o iwm.o lock.o: ../InterViews/sensor.h
desktop.o dispatch.o iwm.o lock.o logo.o: ../InterViews/shape.h
desktop.o dispatch.o iwm.o lock.o: ../InterViews/transformer.h
logo.o: ../InterViews/transformer.h
desktop.o dispatch.o iwm.o lock.o: ../InterViews/world.h
desktop.o dispatch.o iwm.o: ../InterViews/worldview.h
desktop.o dispatch.o iwm.o: ../desktop.h
desktop.o dispatch.o iwm.o: ../dispatch.h
desktop.o dispatch.o iwm.o lock.o: ../lock.h
desktop.o dispatch.o iwm.o logo.o: ../logo.h
desktop.o rubber.o: ../rubber.h
desktop.o dispatch.o iwm.o: /usr/include/CC/stdio.h
desktop.o dispatch.o: /usr/include/CC/string.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
