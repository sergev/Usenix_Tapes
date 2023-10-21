HDR = mongo.h
OBJ = axes.o fecvt.o hash.o input.o ioctl.o mongo.o output.o printer.o 
CFLAGS = -O -DPRINTER

mongo: $(HDR) $(OBJ)
	cc $(CFLAGS) -o mongo $(OBJ) -ltam -ltermcap -lcvdi
