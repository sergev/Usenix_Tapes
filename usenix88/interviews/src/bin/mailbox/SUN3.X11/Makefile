#
# mailbox - a tool to show incoming mail
#

I = ../../../..
T = ../../..
M = SUN3
X = X11
D = ${M}.${X}

AOUT = a.out
DEST = ${I}/bin/${D}/mailbox

CC = CC
CFLAGS = -g -I..
LIBS = ${T}/lib/${D}/libInterViews.a
EXTLIBS = -l${X} -lm

LD = CC
LDFLAGS =

OBJS = mailbox.o

${AOUT}: ${OBJS} ${LIBS}
	${LD} ${LDFLAGS} ${OBJS} ${LIBS} ${EXTLIBS} -o ${AOUT}

install: ${AOUT}
	install -s ${AOUT} ${DEST}

mailbox.o: ../mailbox.c
	${CC} ${CFLAGS} -c ../mailbox.c

clean:
	rm -f a.out core make.out ${OBJS}

depend:
	mkdepend ${CFLAGS} -I/usr/include/CC ${OBJS}
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
mailbox.o: /usr/include/CC/ctype.h
mailbox.o: /usr/include/CC/signal.h
mailbox.o: /usr/include/CC/stdio.h
mailbox.o: /usr/include/CC/string.h
mailbox.o: /usr/include/CC/sys/stat.h
mailbox.o: /usr/include/InterViews/border.h
mailbox.o: /usr/include/InterViews/box.h
mailbox.o: /usr/include/InterViews/defs.h
mailbox.o: /usr/include/InterViews/frame.h
mailbox.o: /usr/include/InterViews/glue.h
mailbox.o: /usr/include/InterViews/interactor.h
mailbox.o: /usr/include/InterViews/paint.h
mailbox.o: /usr/include/InterViews/painter.h
mailbox.o: /usr/include/InterViews/resource.h
mailbox.o: /usr/include/InterViews/scene.h
mailbox.o: /usr/include/InterViews/sensor.h
mailbox.o: /usr/include/InterViews/shape.h
mailbox.o: /usr/include/InterViews/transformer.h
mailbox.o: /usr/include/InterViews/world.h
mailbox.o: /usr/include/machine/param.h
mailbox.o: /usr/include/sys/param.h
mailbox.o: /usr/include/sys/signal.h
mailbox.o: /usr/include/sys/sysmacros.h
mailbox.o: /usr/include/sys/types.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
