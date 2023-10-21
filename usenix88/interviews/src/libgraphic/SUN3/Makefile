#
# InterViews structured graphics library
#

LIB = libgraphic.a
DEST = ../../../lib/SUN3/${LIB}

CC = CC
CFLAGS = -I..

OBJS = \
    base.o cache.o damage.o ellipses.o file.o \
    geomobjs.o grblock.o grconstruct.o hash.o instance.o \
    label.o lines.o objman.o \
    persistent.o picture.o polygons.o ppaint.o \
    ref.o reflist.o splines.o

${LIB}: ${OBJS}
	@rm -f ${LIB}
	@echo "building ${LIB}"
	@ar cr ${LIB} ${OBJS}
	ranlib ${LIB}

install: ${LIB}
	install -c ${LIB} ${DEST}
	ranlib ${DEST}

base.o: ../base.c
	${CC} -g -c ${CFLAGS} ../base.c

cache.o: ../cache.c
	${CC} -c ${CFLAGS} ../cache.c

damage.o: ../damage.c
	${CC} -g -c ${CFLAGS} ../damage.c

ellipses.o: ../ellipses.c
	${CC} -c ${CFLAGS} ../ellipses.c

file.o: ../file.c
	${CC} -c ${CFLAGS} ../file.c

geomobjs.o: ../geomobjs.c
	${CC} -c ${CFLAGS} ../geomobjs.c

grblock.o: ../grblock.c
	${CC} -g -c ${CFLAGS} ../grblock.c

grconstruct.o: ../grconstruct.c
	${CC} -c ${CFLAGS} ../grconstruct.c

hash.o: ../hash.c
	${CC} -g -c ${CFLAGS} ../hash.c

instance.o: ../instance.c
	${CC} -c ${CFLAGS} ../instance.c

label.o: ../label.c
	${CC} -g -c ${CFLAGS} ../label.c

lines.o: ../lines.c
	${CC} -g -c ${CFLAGS} ../lines.c

objman.o: ../objman.c
	${CC} -c ${CFLAGS} ../objman.c

persistent.o: ../persistent.c
	${CC} -c ${CFLAGS} ../persistent.c

picture.o: ../picture.c
	${CC} -g -c ${CFLAGS} ../picture.c

polygons.o: ../polygons.c
	${CC} -g -c ${CFLAGS} ../polygons.c

ppaint.o: ../ppaint.c
	${CC} -c ${CFLAGS} ../ppaint.c

ref.o: ../ref.c
	${CC} -c ${CFLAGS} ../ref.c

reflist.o: ../reflist.c
	${CC} -c ${CFLAGS} ../reflist.c

splines.o: ../splines.c
	${CC} -g -c ${CFLAGS} ../splines.c

clean:
	rm -f ${LIB} ${OBJS} a.out core make.out

depend:
	mkdepend ${CFLAGS} ${OBJS}
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
base.o damage.o ellipses.o grblock.o: ../InterViews/Graphic/base.h
grconstruct.o hash.o instance.o: ../InterViews/Graphic/base.h
label.o lines.o picture.o polygons.o: ../InterViews/Graphic/base.h
splines.o: ../InterViews/Graphic/base.h
cache.o objman.o: ../InterViews/Graphic/cache.h
base.o cache.o damage.o ellipses.o: ../InterViews/Graphic/classes.h
geomobjs.o grblock.o grconstruct.o: ../InterViews/Graphic/classes.h
hash.o instance.o label.o lines.o: ../InterViews/Graphic/classes.h
objman.o persistent.o picture.o: ../InterViews/Graphic/classes.h
polygons.o ppaint.o ref.o: ../InterViews/Graphic/classes.h
reflist.o splines.o: ../InterViews/Graphic/classes.h
damage.o: ../InterViews/Graphic/damage.h
ellipses.o grconstruct.o: ../InterViews/Graphic/ellipses.h
base.o cache.o damage.o ellipses.o: ../InterViews/Graphic/file.h
file.o geomobjs.o grblock.o: ../InterViews/Graphic/file.h
grconstruct.o hash.o instance.o: ../InterViews/Graphic/file.h
label.o lines.o objman.o persistent.o: ../InterViews/Graphic/file.h
picture.o polygons.o ppaint.o ref.o: ../InterViews/Graphic/file.h
reflist.o splines.o: ../InterViews/Graphic/file.h
base.o damage.o ellipses.o: ../InterViews/Graphic/geomobjs.h
geomobjs.o grblock.o: ../InterViews/Graphic/geomobjs.h
grconstruct.o hash.o instance.o: ../InterViews/Graphic/geomobjs.h
label.o lines.o picture.o: ../InterViews/Graphic/geomobjs.h
polygons.o splines.o: ../InterViews/Graphic/geomobjs.h
grblock.o: ../InterViews/Graphic/grblock.h
base.o damage.o ellipses.o: ../InterViews/Graphic/grclasses.h
geomobjs.o grblock.o: ../InterViews/Graphic/grclasses.h
grconstruct.o hash.o instance.o: ../InterViews/Graphic/grclasses.h
label.o lines.o picture.o: ../InterViews/Graphic/grclasses.h
polygons.o ppaint.o splines.o: ../InterViews/Graphic/grclasses.h
grconstruct.o: ../InterViews/Graphic/grconstruct.h
base.o hash.o: ../InterViews/Graphic/hash.h
grconstruct.o instance.o: ../InterViews/Graphic/instance.h
grconstruct.o label.o: ../InterViews/Graphic/label.h
grconstruct.o lines.o polygons.o: ../InterViews/Graphic/lines.h
splines.o: ../InterViews/Graphic/lines.h
base.o cache.o damage.o ellipses.o: ../InterViews/Graphic/objman.h
geomobjs.o grblock.o grconstruct.o: ../InterViews/Graphic/objman.h
hash.o instance.o label.o lines.o: ../InterViews/Graphic/objman.h
objman.o persistent.o picture.o: ../InterViews/Graphic/objman.h
polygons.o ppaint.o ref.o reflist.o: ../InterViews/Graphic/objman.h
splines.o: ../InterViews/Graphic/objman.h
base.o cache.o damage.o: ../InterViews/Graphic/persistent.h
ellipses.o geomobjs.o grblock.o: ../InterViews/Graphic/persistent.h
grconstruct.o hash.o instance.o: ../InterViews/Graphic/persistent.h
label.o lines.o objman.o: ../InterViews/Graphic/persistent.h
persistent.o picture.o: ../InterViews/Graphic/persistent.h
polygons.o ppaint.o ref.o: ../InterViews/Graphic/persistent.h
reflist.o splines.o: ../InterViews/Graphic/persistent.h
grconstruct.o picture.o: ../InterViews/Graphic/picture.h
grconstruct.o polygons.o: ../InterViews/Graphic/polygons.h
splines.o: ../InterViews/Graphic/polygons.h
base.o damage.o ellipses.o: ../InterViews/Graphic/ppaint.h
grblock.o grconstruct.o hash.o: ../InterViews/Graphic/ppaint.h
instance.o label.o lines.o: ../InterViews/Graphic/ppaint.h
picture.o polygons.o ppaint.o: ../InterViews/Graphic/ppaint.h
splines.o: ../InterViews/Graphic/ppaint.h
base.o cache.o damage.o ellipses.o: ../InterViews/Graphic/ref.h
geomobjs.o grblock.o grconstruct.o: ../InterViews/Graphic/ref.h
hash.o instance.o label.o lines.o: ../InterViews/Graphic/ref.h
objman.o picture.o polygons.o ppaint.o: ../InterViews/Graphic/ref.h
ref.o reflist.o splines.o: ../InterViews/Graphic/ref.h
base.o damage.o ellipses.o: ../InterViews/Graphic/reflist.h
geomobjs.o grblock.o grconstruct.o: ../InterViews/Graphic/reflist.h
hash.o instance.o label.o lines.o: ../InterViews/Graphic/reflist.h
objman.o picture.o polygons.o: ../InterViews/Graphic/reflist.h
ppaint.o reflist.o splines.o: ../InterViews/Graphic/reflist.h
grconstruct.o splines.o: ../InterViews/Graphic/splines.h
base.o damage.o ellipses.o geomobjs.o: ../InterViews/Graphic/util.h
grblock.o grconstruct.o hash.o: ../InterViews/Graphic/util.h
instance.o label.o lines.o picture.o: ../InterViews/Graphic/util.h
polygons.o splines.o: ../InterViews/Graphic/util.h
base.o damage.o grblock.o instance.o: ../InterViews/canvas.h
picture.o: ../InterViews/canvas.h
base.o cache.o damage.o ellipses.o file.o: ../InterViews/defs.h
geomobjs.o grblock.o grconstruct.o hash.o: ../InterViews/defs.h
instance.o label.o lines.o objman.o: ../InterViews/defs.h
persistent.o picture.o polygons.o ppaint.o: ../InterViews/defs.h
ref.o reflist.o splines.o: ../InterViews/defs.h
grblock.o ppaint.o: ../InterViews/interactor.h
base.o damage.o ellipses.o grblock.o: ../InterViews/paint.h
grconstruct.o hash.o instance.o label.o: ../InterViews/paint.h
lines.o picture.o polygons.o ppaint.o: ../InterViews/paint.h
splines.o: ../InterViews/paint.h
base.o damage.o ellipses.o grblock.o: ../InterViews/painter.h
grconstruct.o hash.o instance.o label.o: ../InterViews/painter.h
lines.o picture.o polygons.o splines.o: ../InterViews/painter.h
grblock.o: ../InterViews/perspective.h
base.o damage.o ellipses.o grblock.o: ../InterViews/resource.h
grconstruct.o hash.o instance.o label.o: ../InterViews/resource.h
lines.o picture.o polygons.o ppaint.o: ../InterViews/resource.h
splines.o: ../InterViews/resource.h
grblock.o ppaint.o: ../InterViews/scene.h
grblock.o: ../InterViews/shape.h
base.o damage.o ellipses.o grblock.o: ../InterViews/transformer.h
grconstruct.o hash.o instance.o: ../InterViews/transformer.h
label.o lines.o picture.o polygons.o: ../InterViews/transformer.h
splines.o: ../InterViews/transformer.h
ppaint.o: ../InterViews/world.h
base.o damage.o ellipses.o geomobjs.o: /usr/include/math.h
grblock.o grconstruct.o hash.o instance.o: /usr/include/math.h
label.o lines.o picture.o polygons.o splines.o: /usr/include/math.h
base.o cache.o damage.o ellipses.o file.o: /usr/include/stdio.h
geomobjs.o grblock.o grconstruct.o hash.o: /usr/include/stdio.h
instance.o label.o lines.o objman.o: /usr/include/stdio.h
persistent.o picture.o polygons.o ppaint.o: /usr/include/stdio.h
ref.o reflist.o splines.o: /usr/include/stdio.h
file.o objman.o ppaint.o: /usr/include/string.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
