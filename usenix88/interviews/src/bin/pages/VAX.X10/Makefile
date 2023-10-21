#
# Text library demo program
#

I = ../../../..
T = ../../..
M = VAX
X = X10
D = ${M}.${X}

AOUT = a.out
DEST = ${I}/bin/${D}/pages

CC = CC
CFLAGS = -g -I..
LIBS = ${T}/libtext/${M}/libtext.a ${T}/lib/${D}/libInterViews.a
EXTLIBS = -l${X} -lm

LD = CC
LDFLAGS =

OBJS = main.o pages.o aux.o script.o stack.o

${AOUT}: ${OBJS} ${LIBS}
	${LD} ${LDFLAGS} ${OBJS} ${LIBS} ${EXTLIBS} -o ${AOUT}

install: ${AOUT}
	install -s ${AOUT} ${DEST}

main.o: ../main.c
	${CC} ${CFLAGS} -c ../main.c

pages.o: ../pages.c
	${CC} ${CFLAGS} -c ../pages.c

aux.o: ../aux.c
	${CC} ${CFLAGS} -c ../aux.c

script.o: ../script.c
	${CC} ${CFLAGS} -c ../script.c

stack.o: ../stack.c
	${CC} ${CFLAGS} -c ../stack.c

clean:
	rm -f a.out core make.out ${OBJS}

depend:
	mkdepend ${CFLAGS} -I/usr/include/CC ${OBJS}
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
aux.o main.o pages.o: ../InterViews/Text/layout.h
aux.o main.o pages.o: ../InterViews/Text/text.h
aux.o main.o pages.o: ../InterViews/Text/textblock.h
aux.o main.o pages.o: ../InterViews/Text/textpainter.h
pages.o: ../InterViews/border.h
pages.o: ../InterViews/box.h
pages.o: ../InterViews/deck.h
aux.o main.o pages.o script.o stack.o: ../InterViews/defs.h
main.o: ../InterViews/frame.h
pages.o: ../InterViews/glue.h
aux.o main.o pages.o: ../InterViews/interactor.h
aux.o main.o pages.o: ../InterViews/paint.h
aux.o main.o pages.o: ../InterViews/painter.h
aux.o main.o pages.o: ../InterViews/resource.h
main.o pages.o: ../InterViews/scene.h
pages.o: ../InterViews/scroller.h
main.o pages.o: ../InterViews/sensor.h
aux.o main.o pages.o: ../InterViews/shape.h
aux.o main.o pages.o: ../InterViews/transformer.h
main.o: ../InterViews/world.h
main.o pages.o: ../pages.h
aux.o script.o: ../script.h
aux.o stack.o: ../stack.h
pages.o script.o: /usr/include/CC/ctype.h
aux.o main.o script.o: /usr/include/CC/stdio.h
main.o script.o: /usr/include/CC/string.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
