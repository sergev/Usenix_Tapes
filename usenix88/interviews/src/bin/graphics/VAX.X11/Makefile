#
# Graphic library demo program
#

I = ../../../..
T = ../../..
M = VAX
X = X11
D = ${M}.${X}

AOUT = a.out
DEST = ${I}/bin/${D}/graphics

CC = CC
CFLAGS = -g -I..
LIBS = ${T}/libgraphic/${M}/libgraphic.a ${T}/lib/${D}/libInterViews.a
EXTLIBS = -l${X} -lm

LD = CC
LDFLAGS =

OBJS = graphics.o

${AOUT}: ${OBJS} ${LIBS}
	${LD} ${LDFLAGS} ${OBJS} ${LIBS} ${EXTLIBS} -o ${AOUT}

install: ${AOUT}
	install -s ${AOUT} ${DEST}

graphics.o: ../graphics.c
	${CC} ${CFLAGS} -c ../graphics.c

clean:
	rm -f a.out core make.out ${OBJS}

depend:
	mkdepend ${CFLAGS} -I/usr/include/CC ${OBJS}
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
graphics.o: ../InterViews/Graphic/base.h
graphics.o: ../InterViews/Graphic/classes.h
graphics.o: ../InterViews/Graphic/ellipses.h
graphics.o: ../InterViews/Graphic/file.h
graphics.o: ../InterViews/Graphic/geomobjs.h
graphics.o: ../InterViews/Graphic/grblock.h
graphics.o: ../InterViews/Graphic/grclasses.h
graphics.o: ../InterViews/Graphic/grconstruct.h
graphics.o: ../InterViews/Graphic/label.h
graphics.o: ../InterViews/Graphic/lines.h
graphics.o: ../InterViews/Graphic/objman.h
graphics.o: ../InterViews/Graphic/persistent.h
graphics.o: ../InterViews/Graphic/picture.h
graphics.o: ../InterViews/Graphic/polygons.h
graphics.o: ../InterViews/Graphic/ppaint.h
graphics.o: ../InterViews/Graphic/ref.h
graphics.o: ../InterViews/Graphic/reflist.h
graphics.o: ../InterViews/Graphic/splines.h
graphics.o: ../InterViews/Graphic/util.h
graphics.o: ../InterViews/banner.h
graphics.o: ../InterViews/border.h
graphics.o: ../InterViews/box.h
graphics.o: ../InterViews/defs.h
graphics.o: ../InterViews/frame.h
graphics.o: ../InterViews/glue.h
graphics.o: ../InterViews/graphic.h
graphics.o: ../InterViews/interactor.h
graphics.o: ../InterViews/paint.h
graphics.o: ../InterViews/painter.h
graphics.o: ../InterViews/resource.h
graphics.o: ../InterViews/rubband.h
graphics.o: ../InterViews/rubrect.h
graphics.o: ../InterViews/scene.h
graphics.o: ../InterViews/scroller.h
graphics.o: ../InterViews/sensor.h
graphics.o: ../InterViews/shape.h
graphics.o: ../InterViews/transformer.h
graphics.o: ../InterViews/world.h
graphics.o: /usr/include/CC/math.h
graphics.o: /usr/include/CC/stdio.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
