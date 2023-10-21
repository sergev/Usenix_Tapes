#
# InterViews library demo program
#

I = ../../../..
T = ../../..
M = SUN3
X = X11

AOUT = a.out
DEST = ${I}/bin/${M}.${X}/squares

CC = CC
CFLAGS = -g -I..
LIBS = ${T}/lib/${M}.${X}/libInterViews.a
EXTLIBS = -l${X} -lm

LD = CC
LDFLAGS =

OBJS = main.o metaview.o sframe.o subject.o view.o

${AOUT}: ${OBJS} ${LIBS}
	${LD} ${LDFLAGS} ${OBJS} ${LIBS} ${EXTLIBS} -o ${AOUT}

install: ${AOUT}
	install -s ${AOUT} ${DEST}

main.o: ../main.c
	${CC} ${CFLAGS} -c ../main.c

metaview.o: ../metaview.c
	${CC} ${CFLAGS} -c ../metaview.c

sframe.o: ../sframe.c
	${CC} ${CFLAGS} -c ../sframe.c

subject.o: ../subject.c
	${CC} ${CFLAGS} -c ../subject.c

view.o: ../view.c
	${CC} ${CFLAGS} -c ../view.c

clean:
	rm -f a.out core make.out ${OBJS}

depend:
	mkdepend ${CFLAGS} -I/usr/include/CC ${OBJS}

# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
sframe.o: ../InterViews/banner.h
sframe.o: ../InterViews/border.h
metaview.o sframe.o: ../InterViews/box.h
metaview.o: ../InterViews/button.h
main.o metaview.o sframe.o subject.o view.o: ../InterViews/defs.h
main.o metaview.o sframe.o: ../InterViews/frame.h
metaview.o sframe.o: ../InterViews/glue.h
main.o metaview.o sframe.o subject.o: ../InterViews/interactor.h
view.o: ../InterViews/interactor.h
main.o metaview.o sframe.o: ../InterViews/menu.h
metaview.o view.o: ../InterViews/paint.h
metaview.o view.o: ../InterViews/painter.h
view.o: ../InterViews/perspective.h
main.o metaview.o sframe.o view.o: ../InterViews/resource.h
main.o metaview.o sframe.o: ../InterViews/scene.h
sframe.o: ../InterViews/scroller.h
main.o metaview.o sframe.o: ../InterViews/sensor.h
metaview.o sframe.o view.o: ../InterViews/shape.h
metaview.o view.o: ../InterViews/transformer.h
main.o metaview.o sframe.o: ../InterViews/world.h
metaview.o sframe.o: ../metaview.h
main.o metaview.o sframe.o: ../sframe.h
main.o sframe.o subject.o view.o: ../subject.h
main.o sframe.o subject.o view.o: ../view.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
