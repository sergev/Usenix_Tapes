#
#  FILE
#
#	Makefile    build and install the pml library
#
#  SYNOPSIS
#
#	make funcs	make version of library in local directory
#	make install	install the library (must be root)
#
#  WARNING
#
#	The order of the modules listed in the "LEVEL<n>" macros
#	is significant since these are the orders in which
#	they will be loaded into the library archive.  Although
#	some machines support randomly ordered libraries, ordering
#	them correctly doesn't hurt...

LIB =		/usr/lib

INC =		/usr/include

CFLAGS =	-O -DIEEE

LEVEL0 =	ldexp.o frexp.o modf.o matherr.o

LEVEL1 =	sign.o mod.o poly.o dabs.o sqrt.o

LEVEL2 =	acos.o acosh.o asin.o asinh.o atan2.o atan.o \
		log10.o tan.o \
		tanh.o sinh.o cosh.o atanh.o \
		log.o sin.o cos.o exp.o max.o min.o

LEVEL3 =	casin.o cacos.o cmult.o catan.o ccosh.o clog.o \
		crcp.o csinh.o csqrt.o ctan.o ctanh.o cexp.o \
		ccos.o csin.o cdiv.o csubt.o cabs.o

LEVEL4 =	cadd.o

OBJ =		$(LEVEL4) $(LEVEL3) $(LEVEL2) $(LEVEL1)

#
#	The default thing to make.
#

default:	libpml.a

libpml.a:	$(OBJ)
		ar cr libpml.a $(OBJ)

$(OBJ) :	pml.h

#
#	Stuff to do installation
#

install :	$(LIB)/libpml.a $(INC)/pmluser.h

$(LIB)/libpml.a:	libpml.a
			cp libpml.a $(LIB)/libpml.a

$(INC)/pmluser.h :	pmluser.h
			cp pmluser.h $(INC)/pmluser.h

#
#	Clean up the directory.
#

clean:
	rm -f *.o *.BAK libpml.a *.tmp *.bak nohup.out
