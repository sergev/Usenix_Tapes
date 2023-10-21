#
# Makefile for UNIX keyed-IO routines.
# 	(c) Ken Wellsch 1985
#
CFLAGS = -O
KLIB = klib.a

SRC =	kioglob.c GetBlk.c GetRec.c MakBlk.c MakNam.c \
	MapBlk.c MapKey.c PutBlk.c PutRec.c \
	closek.c creatk.c dupk.c openk.c readk.c writek.c 

OBJ =	GetBlk.o GetRec.o MakBlk.o MakNam.o \
	MapBlk.o MapKey.o PutBlk.o PutRec.o \
	closek.o creatk.o kioglob.o openk.o \
	dupk.o readk.o writek.o 

HEADER = kio.h

all: $(KLIB)

$(SRC): $(HEADER)
	touch $@

$(KLIB): $(SRC)
	-ar x $(KLIB)
	-cc $(CFLAGS) -c $?
	rm -f $(KLIB)
	-ar ru $(KLIB) $(OBJ)
	rm -f *.o
	ranlib $(KLIB)

clean:
	rm *.o

backup:
	cp Makefile $(HEADER) $(SRC) bkup

print:
	lpr -Phw -p -J "Keyed I/O Lib" Makefile $(HEADER) $(SRC)

lint:
	lint $(SRC)
