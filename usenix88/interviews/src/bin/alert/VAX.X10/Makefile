#
# alert - builds a dialog box from standard input text
#

I = ../../../..
T = ../../..
M = VAX
X = X10

AOUT = a.out
DEST = ${I}/bin/${M}.${X}/alert

CC = CC
CFLAGS = -g -I..
LIBS = ${T}/lib/${M}.${X}/libInterViews.a
EXTLIBS = -l${X} -lm

LD = CC
LDFLAGS =

OBJS = alert.o

${AOUT}: ${OBJS} ${LIBS}
	${LD} ${LDFLAGS} ${OBJS} ${LIBS} ${EXTLIBS} -o ${AOUT}

install: ${AOUT}
	install -s ${AOUT} ${DEST}

alert.o: ../alert.c
	${CC} ${CFLAGS} -c ../alert.c

clean:
	rm -f a.out core make.out ${OBJS}

depend:
	mkdepend ${CFLAGS} -I/usr/include/CC ${OBJS}
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
alert.o: ../InterViews/box.h
alert.o: ../InterViews/button.h
alert.o: ../InterViews/defs.h
alert.o: ../InterViews/frame.h
alert.o: ../InterViews/glue.h
alert.o: ../InterViews/interactor.h
alert.o: ../InterViews/paint.h
alert.o: ../InterViews/painter.h
alert.o: ../InterViews/resource.h
alert.o: ../InterViews/scene.h
alert.o: ../InterViews/sensor.h
alert.o: ../InterViews/shape.h
alert.o: ../InterViews/transformer.h
alert.o: ../InterViews/world.h
alert.o: /usr/include/CC/stdio.h
alert.o: /usr/include/CC/string.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
