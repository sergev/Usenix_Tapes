#
# Backward compatible library for old InterViews 1.0 interfaces
#

X = X10

VPATH = :..:

LIB = imagelib.o

CC = CC
CFLAGS = -I. -D${X}

OBJS = vdi.o image.o

${LIB}: ${OBJS}
	@rm -f ${LIB}
	@echo "building ${LIB}"
	@ld -r -o ${LIB} ${OBJS}

${OBJS}:
	${CC} -c ${CFLAGS} $*.c

clean:
	rm -f ${LIB} ${OBJS} a.out core make.out

depend:
	mkdepend ${CFLAGS} ${OBJS}
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
image.o: ./InterViews/X10/Xdefs.h
image.o: ./InterViews/X10/Xinput.h
image.o: ./InterViews/X10/Xoutput.h
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
image.o vdi.o: ./image.h
image.o: ./table.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
