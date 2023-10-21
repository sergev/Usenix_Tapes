#
# Makefile for the Adventure Compiler.
# 	(c) Ken Wellsch 1985
#
CFLAGS = -O
KLIB = ../kio/klib.a
MLIB = mlib.a

MAIN = munge.c

SRC =	buffers.c compile.c fio.c glob.c \
	majors.c parse.c symtab.c util.c

OBJ =	buffers.o compile.o fio.o glob.o \
	majors.o parse.o symtab.o util.o

HEADER = mdefs.h

munge: $(MAIN) $(MLIB)
	cc $(CFLAGS) $(MAIN) mlib.a $(KLIB) -o munge
	size munge

$(SRC) $(MAIN):: mdefs.h
	touch $@

$(MLIB): $(SRC)
	-ar x $(MLIB)
	-cc $(CFLAGS) -c $?
	rm -f $(MLIB)
	-ar ru $(MLIB) $(OBJ)
	rm -f *.o
	ranlib $(MLIB)

clean:
	rm *.o

backup:
	cp Makefile $(HEADER) $(MAIN) $(SRC) bkup

print:
	lpr -Phw -p -J "Munge" Makefile $(HEADER) $(MAIN) $(SRC)

lint:
	lint $(SRC) $(MAIN)
