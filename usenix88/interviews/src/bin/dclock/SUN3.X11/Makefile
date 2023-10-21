#
# scalable digital clock
#

I = ../../../..
T = ../../..
M = SUN3
X = X11

AOUT = a.out
DEST = ${I}/bin/${M}.${X}/dclock

CC = CC
CFLAGS = -g -I.. -DBACKGROUND
LIBS = ${T}/lib/${M}.${X}/libInterViews.a
EXTLIBS = -l${X} -lm

LD = CC
LDFLAGS =

OBJS = data.o dclock.o dface.o digit.o segment.o clocktime.o

${AOUT}: ${OBJS} ${LIBS}
	${LD} ${LDFLAGS} ${OBJS} ${LIBS} ${EXTLIBS} -o ${AOUT}

install: ${AOUT}
	install -s ${AOUT} ${DEST}

data.o: ../data.c
	${CC} ${CFLAGS} -c ../data.c

dclock.o: ../dclock.c
	${CC} ${CFLAGS} -c ../dclock.c

dface.o: ../dface.c
	${CC} ${CFLAGS} -c ../dface.c

digit.o: ../digit.c
	${CC} ${CFLAGS} -c ../digit.c

segment.o: ../segment.c
	${CC} ${CFLAGS} -c ../segment.c

clocktime.o: ../clocktime.c
	${CC} ${CFLAGS} -c ../clocktime.c

clean:
	rm -f a.out core make.out ${OBJS}

depend:
	mkdepend ${CFLAGS} -I/usr/include/CC ${OBJS}
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
data.o dclock.o dface.o digit.o segment.o: ../InterViews/banner.h
data.o dclock.o dface.o digit.o segment.o: ../InterViews/border.h
data.o dclock.o dface.o digit.o segment.o: ../InterViews/box.h
data.o dclock.o dface.o digit.o segment.o: ../InterViews/canvas.h
data.o dclock.o dface.o digit.o segment.o: ../InterViews/defs.h
data.o dclock.o dface.o digit.o segment.o: ../InterViews/frame.h
data.o dclock.o dface.o digit.o: ../InterViews/interactor.h
segment.o: ../InterViews/interactor.h
data.o dclock.o dface.o digit.o segment.o: ../InterViews/paint.h
data.o dclock.o dface.o digit.o segment.o: ../InterViews/painter.h
data.o dclock.o dface.o digit.o segment.o: ../InterViews/resource.h
data.o dclock.o dface.o digit.o segment.o: ../InterViews/scene.h
data.o dclock.o dface.o digit.o segment.o: ../InterViews/sensor.h
data.o dclock.o dface.o digit.o segment.o: ../InterViews/shape.h
data.o dclock.o dface.o digit.o: ../InterViews/transformer.h
segment.o: ../InterViews/transformer.h
data.o dclock.o dface.o digit.o segment.o: ../InterViews/world.h
clocktime.o dface.o: ../clocktime.h
data.o dclock.o dface.o digit.o segment.o: ../data.h
data.o dclock.o dface.o digit.o segment.o: ../dclock.h
dclock.o dface.o: ../dface.h
dface.o digit.o: ../digit.h
data.o digit.o segment.o: ../segment.h
dclock.o: /usr/include/CC/stdio.h
clocktime.o data.o dclock.o dface.o: /usr/include/CC/string.h
digit.o segment.o: /usr/include/CC/string.h
clocktime.o: /usr/include/CC/time.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
