CFLAGS = -O -I..
DOTOS = bdinit.o clearflg.o copy.o glb.o groups.o grpop.o gsize.o holes.o \
	legality.o lfs.o liberties.o makemove.o numcap.o stoc.o
DOTCS = bdinit.c clearflg.c copy.c glb.c groups.c grpop.c gsize.c holes.c \
	legality.c lfs.c liberties.c makemove.c numcap.c stoc.c
LIB = ../gosub.a

default: $(LIB)
	ranlib $(LIB)
	@echo "$(LIB) is up to date unless there were diagnostics."

$(LIB):: $(DOTCS)
	-$(CC) $(CFLAGS) -c $?
	-ar uv $(LIB) *.o
	-rm -f *.o
