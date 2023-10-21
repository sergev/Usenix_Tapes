#
#  Makefile for suntools alarmer
#
BIN	= alarm
SRC	= $(BIN).c
CC	= cc
COPTS	= 
ICON	= -DICON_DIR="/usr/local/lib/icons"
STRIP	= strip

$(BIN)	: $(SRC)
	cc -o $@ $(ICON) $(CFLAGS) $?
	$(STRIP) $@
