
#	makefile for CIRES fortran mathematical library

FFLAGS	=	-O -I2

OBJECTS	=	bsint.o dgelg.o lfft.o mfft.o nfac.o ffttwo.o fft.o spline.o\
                realtr.o chol.o prol.o hann.o krner.o ai0.o qr.o qrcov.o sft.o\
                ur2.o


Cm:	$(OBJECTS)
	ar cr Cm $(OBJECTS)
	ranlib Cm
	rm *.o
	mv Cm /usr/lib/libCm.a

