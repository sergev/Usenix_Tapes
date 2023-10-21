#
#  This is the MAKEFILE that work on my machine (TRS-80 Model 16 running
#  XENIX).  Check to see if they're correct for yours.  Don't forget to
#  turn the leading spaces into tabs!!!
#

BINARY = /mdr/bin/dialup

OBJECTS = main.o get_service.o

LIBRARIES = -lc

CFLAGS = -O

LDFLAGS = -n -s -x

$(BINARY): $(OBJECTS)
	@echo loading $(BINARY)
	@ld -o $(BINARY) $(LDFLAGS) /lib/crt0.o $(OBJECTS) $(LIBRARIES)
	@chmod 771 $(BINARY)
