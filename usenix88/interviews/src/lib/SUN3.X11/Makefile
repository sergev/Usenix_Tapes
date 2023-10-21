#
# InterViews library
#

LIB = libInterViews.a
DEST = ../../../lib/SUN3.X11/${LIB}

CC = CC
CFLAGS = -I.. -DX11

OBJS = \
    adjuster.o banner.o bitmap.o border.o box.o button.o \
    canvas.o cursor.o deck.o frame.o \
    glue.o graphics.o interactor.o menu.o \
    paint.o painter.o panner.o perspective.o \
    reqerr.o rubband.o rubcurve.o rubline.o rubrect.o \
    scene.o scroller.o sensor.o shape.o spline.o \
    table.o transformer.o viewport.o world.o worldview.o

${LIB}: ${OBJS}
	@rm -f ${LIB}
	@echo "building ${LIB}"
	@ar cr ${LIB} ${OBJS}
	ranlib ${LIB}

install: ${LIB}
	install -c ${LIB} ${DEST}
	ranlib ${DEST}

adjuster.o: ../adjuster.c
	${CC} -c ${CFLAGS} ../adjuster.c

banner.o: ../banner.c
	${CC} -c ${CFLAGS} ../banner.c

bitmap.o: ../bitmap.c
	${CC} -c ${CFLAGS} ../bitmap.c

border.o: ../border.c
	${CC} -c ${CFLAGS} ../border.c

box.o: ../box.c
	${CC} -c ${CFLAGS} ../box.c

button.o: ../button.c
	${CC} -c ${CFLAGS} ../button.c

canvas.o: ../canvas.c
	${CC} -c ${CFLAGS} ../canvas.c

cursor.o: ../cursor.c
	${CC} -c ${CFLAGS} ../cursor.c

deck.o: ../deck.c
	${CC} -c ${CFLAGS} ../deck.c

frame.o: ../frame.c
	${CC} -c ${CFLAGS} ../frame.c

glue.o: ../glue.c
	${CC} -c ${CFLAGS} ../glue.c

graphics.o: ../graphics.c
	${CC} -c ${CFLAGS} ../graphics.c

interactor.o: ../interactor.c
	${CC} -c ${CFLAGS} ../interactor.c

menu.o: ../menu.c
	${CC} -c ${CFLAGS} ../menu.c

paint.o: ../paint.c
	${CC} -c ${CFLAGS} ../paint.c

painter.o: ../painter.c
	${CC} -c ${CFLAGS} ../painter.c

panner.o: ../panner.c
	${CC} -c ${CFLAGS} ../panner.c

perspective.o: ../perspective.c
	${CC} -c ${CFLAGS} ../perspective.c

reqerr.o: ../reqerr.c
	${CC} -c ${CFLAGS} ../reqerr.c

rubband.o: ../rubband.c
	${CC} -c ${CFLAGS} ../rubband.c

rubcurve.o: ../rubcurve.c
	${CC} -c ${CFLAGS} ../rubcurve.c

rubline.o: ../rubline.c
	${CC} -c ${CFLAGS} ../rubline.c

rubrect.o: ../rubrect.c
	${CC} -c ${CFLAGS} ../rubrect.c

scene.o: ../scene.c
	${CC} -c ${CFLAGS} ../scene.c

scroller.o: ../scroller.c
	${CC} -c ${CFLAGS} ../scroller.c

sensor.o: ../sensor.c
	${CC} -c ${CFLAGS} ../sensor.c

shape.o: ../shape.c
	${CC} -c ${CFLAGS} ../shape.c

spline.o: ../spline.c
	${CC} -c ${CFLAGS} ../spline.c

table.o: ../table.c
	${CC} -c ${CFLAGS} ../table.c

transformer.o: ../transformer.c
	${CC} -c ${CFLAGS} ../transformer.c

viewport.o: ../viewport.c
	${CC} -c ${CFLAGS} ../viewport.c

world.o: ../world.c
	${CC} -c ${CFLAGS} ../world.c

worldview.o: ../worldview.c
	${CC} -c ${CFLAGS} ../worldview.c

clean:
	rm -f ${LIB} ${OBJS} a.out core make.out

depend:
	mkdepend ${CFLAGS} ${OBJS}
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
cursor.o paint.o: ../InterViews/X11/Xcolor.h
cursor.o interactor.o scene.o: ../InterViews/X11/Xcursor.h
bitmap.o canvas.o cursor.o graphics.o: ../InterViews/X11/Xdefs.h
interactor.o paint.o painter.o reqerr.o: ../InterViews/X11/Xdefs.h
rubband.o scene.o sensor.o spline.o: ../InterViews/X11/Xdefs.h
world.o worldview.o: ../InterViews/X11/Xdefs.h
graphics.o paint.o: ../InterViews/X11/Xfont.h
paint.o: ../InterViews/X11/Ximage.h
canvas.o interactor.o reqerr.o scene.o: ../InterViews/X11/Xinput.h
sensor.o world.o worldview.o: ../InterViews/X11/Xinput.h
bitmap.o canvas.o cursor.o graphics.o: ../InterViews/X11/Xoutput.h
interactor.o paint.o painter.o: ../InterViews/X11/Xoutput.h
rubband.o scene.o spline.o world.o: ../InterViews/X11/Xoutput.h
interactor.o sensor.o world.o: ../InterViews/X11/Xutil.h
worldview.o: ../InterViews/X11/Xutil.h
bitmap.o canvas.o interactor.o scene.o: ../InterViews/X11/Xwindow.h
sensor.o world.o worldview.o: ../InterViews/X11/Xwindow.h
cursor.o paint.o: ../InterViews/Xcolor.h
cursor.o interactor.o scene.o: ../InterViews/Xcursor.h
bitmap.o canvas.o cursor.o graphics.o: ../InterViews/Xdefs.h
interactor.o paint.o painter.o reqerr.o: ../InterViews/Xdefs.h
rubband.o scene.o sensor.o spline.o world.o: ../InterViews/Xdefs.h
worldview.o: ../InterViews/Xdefs.h
graphics.o paint.o: ../InterViews/Xfont.h
paint.o: ../InterViews/Ximage.h
canvas.o interactor.o reqerr.o scene.o: ../InterViews/Xinput.h
sensor.o world.o worldview.o: ../InterViews/Xinput.h
bitmap.o canvas.o cursor.o graphics.o: ../InterViews/Xoutput.h
interactor.o paint.o painter.o rubband.o: ../InterViews/Xoutput.h
scene.o spline.o world.o: ../InterViews/Xoutput.h
interactor.o sensor.o world.o worldview.o: ../InterViews/Xutil.h
bitmap.o canvas.o interactor.o scene.o: ../InterViews/Xwindow.h
sensor.o world.o worldview.o: ../InterViews/Xwindow.h
adjuster.o panner.o: ../InterViews/adjuster.h
banner.o frame.o: ../InterViews/banner.h
adjuster.o bitmap.o paint.o: ../InterViews/bitmap.h
border.o frame.o panner.o: ../InterViews/border.h
box.o frame.o menu.o panner.o: ../InterViews/box.h
button.o: ../InterViews/button.h
bitmap.o canvas.o frame.o graphics.o: ../InterViews/canvas.h
interactor.o painter.o scene.o spline.o: ../InterViews/canvas.h
viewport.o world.o worldview.o: ../InterViews/canvas.h
cursor.o interactor.o scene.o scroller.o: ../InterViews/cursor.h
worldview.o: ../InterViews/cursor.h
deck.o: ../InterViews/deck.h
adjuster.o banner.o bitmap.o border.o box.o: ../InterViews/defs.h
button.o canvas.o cursor.o deck.o frame.o: ../InterViews/defs.h
glue.o graphics.o interactor.o menu.o paint.o: ../InterViews/defs.h
painter.o panner.o perspective.o reqerr.o: ../InterViews/defs.h
rubband.o rubcurve.o rubline.o rubrect.o: ../InterViews/defs.h
scene.o scroller.o sensor.o shape.o spline.o: ../InterViews/defs.h
table.o transformer.o viewport.o world.o: ../InterViews/defs.h
worldview.o: ../InterViews/defs.h
frame.o menu.o: ../InterViews/frame.h
deck.o glue.o panner.o: ../InterViews/glue.h
adjuster.o banner.o border.o box.o: ../InterViews/interactor.h
button.o canvas.o deck.o frame.o glue.o: ../InterViews/interactor.h
interactor.o menu.o paint.o panner.o: ../InterViews/interactor.h
perspective.o scene.o scroller.o: ../InterViews/interactor.h
sensor.o viewport.o world.o worldview.o: ../InterViews/interactor.h
menu.o: ../InterViews/menu.h
adjuster.o banner.o bitmap.o border.o: ../InterViews/paint.h
button.o canvas.o cursor.o frame.o glue.o: ../InterViews/paint.h
graphics.o interactor.o menu.o paint.o: ../InterViews/paint.h
painter.o panner.o rubband.o rubcurve.o: ../InterViews/paint.h
rubline.o rubrect.o scene.o scroller.o: ../InterViews/paint.h
spline.o viewport.o world.o: ../InterViews/paint.h
adjuster.o banner.o border.o button.o: ../InterViews/painter.h
frame.o glue.o graphics.o interactor.o: ../InterViews/painter.h
menu.o paint.o painter.o panner.o: ../InterViews/painter.h
rubband.o rubcurve.o rubline.o rubrect.o: ../InterViews/painter.h
scene.o scroller.o spline.o viewport.o: ../InterViews/painter.h
world.o: ../InterViews/painter.h
panner.o: ../InterViews/panner.h
adjuster.o deck.o panner.o: ../InterViews/perspective.h
perspective.o scroller.o viewport.o: ../InterViews/perspective.h
reqerr.o: ../InterViews/reqerr.h
adjuster.o banner.o bitmap.o border.o: ../InterViews/resource.h
button.o canvas.o cursor.o frame.o glue.o: ../InterViews/resource.h
graphics.o interactor.o menu.o paint.o: ../InterViews/resource.h
painter.o panner.o rubband.o rubcurve.o: ../InterViews/resource.h
rubline.o rubrect.o scene.o scroller.o: ../InterViews/resource.h
sensor.o spline.o transformer.o: ../InterViews/resource.h
viewport.o world.o worldview.o: ../InterViews/resource.h
panner.o rubband.o rubcurve.o rubline.o: ../InterViews/rubband.h
rubrect.o scene.o scroller.o: ../InterViews/rubband.h
rubcurve.o: ../InterViews/rubcurve.h
rubline.o: ../InterViews/rubline.h
panner.o rubrect.o scene.o scroller.o: ../InterViews/rubrect.h
box.o deck.o frame.o interactor.o menu.o: ../InterViews/scene.h
paint.o panner.o scene.o sensor.o viewport.o: ../InterViews/scene.h
world.o worldview.o: ../InterViews/scene.h
scroller.o: ../InterViews/scroller.h
adjuster.o button.o frame.o interactor.o: ../InterViews/sensor.h
menu.o panner.o scene.o scroller.o sensor.o: ../InterViews/sensor.h
worldview.o: ../InterViews/sensor.h
adjuster.o banner.o border.o box.o button.o: ../InterViews/shape.h
deck.o frame.o glue.o interactor.o menu.o: ../InterViews/shape.h
panner.o scene.o scroller.o shape.o: ../InterViews/shape.h
viewport.o world.o worldview.o: ../InterViews/shape.h
adjuster.o banner.o bitmap.o border.o: ../InterViews/transformer.h
button.o frame.o glue.o graphics.o: ../InterViews/transformer.h
interactor.o menu.o paint.o painter.o: ../InterViews/transformer.h
panner.o rubband.o rubcurve.o: ../InterViews/transformer.h
rubline.o rubrect.o scene.o scroller.o: ../InterViews/transformer.h
spline.o transformer.o viewport.o: ../InterViews/transformer.h
world.o: ../InterViews/transformer.h
viewport.o: ../InterViews/viewport.h
menu.o paint.o sensor.o world.o worldview.o: ../InterViews/world.h
interactor.o worldview.o: ../InterViews/worldview.h
graphics.o painter.o spline.o: ../graphics.h
canvas.o interactor.o scene.o sensor.o table.o world.o: ../table.h
transformer.o: /usr/include/math.h
world.o: /usr/include/stdio.h
graphics.o paint.o sensor.o: /usr/include/string.h
interactor.o: /usr/include/sys/time.h
interactor.o: /usr/include/time.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
