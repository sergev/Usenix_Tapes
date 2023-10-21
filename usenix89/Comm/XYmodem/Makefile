OBJECTS = xmodem.o getput.o misc.o send.o receive.o batch.o
CFLAGS = -O

xmodem: $(OBJECTS)
	cc $(CFLAGS) $(OBJECTS) -o xmodem

$(OBJECTS): xmodem.h

print: 
	lpr -p -Pvmslp xmodem.h xmodem.c getput.c receive.c send.c batch.c misc.c Makefile
