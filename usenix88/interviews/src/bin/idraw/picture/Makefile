.SUFFIXES: .mod .def .c .o

NAME = picture
DESTLIB = picturelib.o

MOD = mod
DEBUGGING = -g
CHECKING = -C
OPTIM = -O
PROFILE = -pg
MODFLAGS = ${OPTIM}

CC = cc
CFLAGS = ${OPTIM}

LD = mod
LDFLAGS = -g

.mod.def:
	@rm -f $*.def
	@rm -f ${NAME}.def
	mmakedefs -f $*.mod $*.def
	@chmod 444 $*.def

.mod.o:
	${MOD} ${MODFLAGS} -c $*.mod

.c.o:
	${CC} ${CFLAGS} -c $*.c

OBJS = 	align.o attribs.o bbox.o draw.o file.o main.o alloc.o objs.o \
	ops.o redraw.o util.o xforms.o ${NAME}.o

MDEFS = align.def attribs.def bbox.def draw.def file.def main.def alloc.def \
	objs.def ops.def redraw.def util.def xforms.def

DEFS = ${MDEFS} ${NAME}.def

${DESTLIB}: ${DEFS} ${OBJS}
	@echo "building picture library"
	@ld -r -o ${DESTLIB} ${OBJS}

defs: ${DEFS}

${NAME}.def: 
	@echo "creating ${NAME}.def"
	@mktrader ${NAME} ${MDEFS}  > ${NAME}.def

clean:
	rm -f ${LIB} ${DEFS} ${OBJS} a.out core make.out

cleandefs:
	rm -f ${DEFS}
