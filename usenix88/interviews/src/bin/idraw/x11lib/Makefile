#
# Backward compatible library for old InterViews 1.0 interfaces
#

X = X11

VPATH = :..:

LIB = imagelib.o

CC = CC
CFLAGS = -I. -I../lib -D${X}

OBJS = vdi.o image.o xstats.o

${LIB}: ${OBJS}
	@rm -f ${LIB}
	ld -r -o ${LIB} ${OBJS}

${OBJS}:
	${CC} -c ${CFLAGS} $*.c

clean:
	rm -f ${LIB} ${OBJS} a.out core make.out

depend:
	mkdepend ${CFLAGS} ${OBJS}
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
image.o vdi.o: ../lib/image.h
image.o: ../lib/table.h
image.o: ./InterViews/X11/Xdefs.h
image.o: ./InterViews/X11/Xinput.h
image.o: ./InterViews/X11/Xoutput.h
image.o: ./InterViews/Xdefs.h
image.o: ./InterViews/Xinput.h
image.o: ./InterViews/Xoutput.h
image.o vdi.o: ./InterViews/canvas.h
image.o vdi.o: ./InterViews/cursor.h
image.o vdi.o: ./InterViews/defs.h
image.o vdi.o: ./InterViews/interactor.h
image.o vdi.o: ./InterViews/paint.h
image.o: ./InterViews/painter.h
image.o vdi.o: ./InterViews/resource.h
image.o: ./InterViews/scene.h
image.o: ./InterViews/sensor.h
image.o: ./InterViews/shape.h
image.o: ./InterViews/transformer.h
image.o: ./InterViews/world.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
