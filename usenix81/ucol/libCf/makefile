
#	makefile for CIRES fortran miscellaneous library

FFLAGS	=	-O -I2
CFLAGS  =	-O

OBJECTS	=	depict.o send12.o rcur12.o sendeg.o chrbig.o \
		raw.o cooked.o realio.o getfd.o ntype.o ctype.o namtyp.o \
		argfcn.o


Cf:	$(OBJECTS)
	ar cr Cf $(OBJECTS)
	ranlib Cf
	rm *.o
	mv Cf /usr/lib/libCf.a

