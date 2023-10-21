.SUFFIXES: .c .mod .def .o

NAME = draw
BIN = ../../../bin

MOD = mod
DEBUGGING = -g
CHECKING = -C
OPTIM = -O
PROFILE = -pg
#MODFLAGS = 
#MODFLAGS = ${OPTIM}
MODFLAGS = ${DEBUGGING}
#MODFLAGS = ${DEBUGGING} ${OPTIM}

CC = CC
CFLAGS = -I.

LD = mod
LDFLAGS = -g

PICTURE = ./picture
MODULES = ./modules
LIB = lib
IVLIB = -lInterViews
LIBS = \
    ${PICTURE}/picturelib.o ${MODULES}/libmodules.a ${LIB}/imagelib.o \
    ${IVLIB} -lXstats -lX -lC

X11LIBS = \
    ${PICTURE}/picturelib.o ${MODULES}/libmodules.a x11${LIB}/imagelib.o \
    ${IVLIB}-X11 -lX11 -lC

.mod.def:
	@rm -f $*.def
	mmakedefs -f $*.mod $*.def
	@chmod 444 $*.def

.mod.o:
	${MOD} ${MODFLAGS} -c $*.mod

.c.o:
	${CC} ${CFLAGS} -c $*.c

DVDEFS=	dvboot.def dvcmds.def dverror.def dvevents.def dvdefs.def \
	dvscroll.def dvobjtools.def dvoptools.def dvselect.def \
	dvupict.def dvwindops.def dvundo.def

DVOBJS=	dvmain.o \
	dvboot.o dvcmds.o dverror.o dvevents.o dvdefs.o \
	dvscroll.o dvobjtools.o dvoptools.o dvselect.o \
	dvupict.o dvwindops.o dvundo.o

DEFS = 	drawing.def drawingView.def ${DVDEFS} \
	cursorset.def dialog.def dirs.def grays.def menu.def pictmacros.def \
	ps.def rubband.def scrollbar.def utilities.def xstats.def

OBJS =  drawing.o drawingView.o ${DVOBJS} \
	cursorset.o dialog.o dirs.o grays.o menu.o pictmacros.o \
	ps.o rubband.o scrollbar.o utilities.o xstats.o

OTHEROBJS = 

${NAME}: pictureLib imageLib ${DEFS} ${OBJS}
	@echo "linking ${NAME}"
	@${LD} ${LDFLAGS} ${OBJS} ${OTHEROBJS} ${LIBS} -o ${NAME}
#	@finished making ${NAME}

X10: ${NAME}

X11: pictureLib x11imageLib ${DEFS} ${OBJS}
	@echo "linking x11${NAME}"
	@${LD} ${LDFLAGS} ${OBJS} ${OTHEROBJS} ${X11LIBS} -o x11${NAME}

pictureLib:
	cd ${PICTURE}; make

imageLib:
	cd ${LIB}; make

x11imageLib:
	cd x11${LIB}; make

drawingView.def: ${DVDEFS}
	@echo "creating drawingView.def"
	@mktrader drawingView ${DVDEFS} > drawingView.def

defs: ${DEFS}

install: ${NAME}
	install -s ${NAME} ${BIN}/VAX.X10/idraw

X10-install: install

X11-install: X11
	install -s x11${NAME} ${BIN}/VAX.X11/idraw

clean:
	'rm' -f *.{o,def}
