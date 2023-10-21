#
# InterViews Text library
#

LIB = libtext.a
DEST = ../../../lib/SUN3/${LIB}

CC = CC
CFLAGS = -I..

.c.o:
	${CC} -c ${CFLAGS} $*.c

OBJS = \
    text.o layout.o textblock.o \
    textviewer.o stringedit.o \
    textpainter.o \
    textbuffer.o terminal.o emulator.o shell.o

${LIB}: ${OBJS}
	@rm -f ${LIB}
	@echo "building ${LIB}"
	@ar cr ${LIB} ${OBJS}
	ranlib ${LIB}

install: ${LIB}
	install -c ${LIB} ${DEST}
	ranlib ${DEST}

text.o: ../text.c
	${CC} -c ${CFLAGS} ../text.c

layout.o: ../layout.c
	${CC} -c ${CFLAGS} ../layout.c

textblock.o: ../textblock.c
	${CC} -c ${CFLAGS} ../textblock.c

textviewer.o: ../textviewer.c
	${CC} -c ${CFLAGS} ../textviewer.c

stringedit.o: ../stringedit.c
	${CC} -c ${CFLAGS} ../stringedit.c

textpainter.o: ../textpainter.c
	${CC} -c ${CFLAGS} ../textpainter.c

textbuffer.o: ../textbuffer.c
	${CC} -c ${CFLAGS} ../textbuffer.c

terminal.o: ../terminal.c
	${CC} -c ${CFLAGS} ../terminal.c

emulator.o: ../emulator.c
	${CC} -c ${CFLAGS} ../emulator.c

shell.o: ../shell.c
	${CC} -c ${CFLAGS} ../shell.c

clean:
	rm -f ${LIB} ${OBJS} a.out core make.out

depend:
	mkdepend ${CFLAGS} -I/usr/include/CC ${OBJS}
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
emulator.o shell.o: ../InterViews/Text/emulator.h
layout.o text.o textblock.o: ../InterViews/Text/layout.h
shell.o: ../InterViews/Text/shell.h
stringedit.o: ../InterViews/Text/stringedit.h
terminal.o: ../InterViews/Text/terminal.h
layout.o text.o: ../InterViews/Text/text.h
layout.o text.o textblock.o: ../InterViews/Text/textblock.h
emulator.o shell.o terminal.o: ../InterViews/Text/textbuffer.h
textbuffer.o: ../InterViews/Text/textbuffer.h
layout.o textblock.o: ../InterViews/Text/textpainter.h
textpainter.o: ../InterViews/Text/textpainter.h
textviewer.o: ../InterViews/Text/textviewer.h
shell.o terminal.o: ../InterViews/banner.h
shell.o terminal.o: ../InterViews/border.h
shell.o terminal.o: ../InterViews/box.h
textbuffer.o: ../InterViews/canvas.h
shell.o stringedit.o terminal.o textblock.o: ../InterViews/cursor.h
emulator.o layout.o shell.o stringedit.o: ../InterViews/defs.h
terminal.o text.o textblock.o textbuffer.o: ../InterViews/defs.h
textpainter.o textviewer.o: ../InterViews/defs.h
shell.o terminal.o: ../InterViews/frame.h
emulator.o layout.o shell.o: ../InterViews/interactor.h
stringedit.o terminal.o text.o: ../InterViews/interactor.h
textblock.o textbuffer.o textviewer.o: ../InterViews/interactor.h
layout.o shell.o stringedit.o terminal.o: ../InterViews/paint.h
textblock.o textbuffer.o textpainter.o: ../InterViews/paint.h
textviewer.o: ../InterViews/paint.h
layout.o shell.o stringedit.o terminal.o: ../InterViews/painter.h
textblock.o textbuffer.o textpainter.o: ../InterViews/painter.h
textviewer.o: ../InterViews/painter.h
textblock.o textviewer.o: ../InterViews/perspective.h
layout.o shell.o stringedit.o terminal.o: ../InterViews/resource.h
textblock.o textbuffer.o textpainter.o: ../InterViews/resource.h
textviewer.o: ../InterViews/resource.h
shell.o stringedit.o terminal.o textblock.o: ../InterViews/scene.h
textviewer.o: ../InterViews/scene.h
layout.o shell.o stringedit.o terminal.o: ../InterViews/sensor.h
textblock.o textviewer.o: ../InterViews/sensor.h
layout.o shell.o stringedit.o terminal.o: ../InterViews/shape.h
text.o textblock.o textbuffer.o textviewer.o: ../InterViews/shape.h
layout.o shell.o stringedit.o: ../InterViews/transformer.h
terminal.o textblock.o textbuffer.o: ../InterViews/transformer.h
textpainter.o textviewer.o: ../InterViews/transformer.h
stringedit.o: /usr/include/CC/ctype.h
shell.o: /usr/include/CC/stdio.h
stringedit.o text.o textviewer.o: /usr/include/CC/string.h
shell.o: /usr/include/sys/ioctl.h
shell.o: /usr/include/sys/ttychars.h
shell.o: /usr/include/sys/ttydev.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
