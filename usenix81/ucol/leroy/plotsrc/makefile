
OBJECTS =	axis.o box.o cfont.o chrdir.o chrsiz.o clear.o\
		drawv.o hpmap.o initt.o line.o ltext.o ltype.o\
		movev.o nplot.o plot1.o pspeed.o selpen.o\
		setdim.o setscl.o stack.o tdraw.o text.o\
		tics.o wait.o xlimit.o xmap.o rawio.o\
		done.o circle.o

CFLAGS = -c
FFLAGS = -c

plib:	$(OBJECTS)
	ar r plib $?
	ranlib plib
