#
#	Makefile for $(BIN)
#
.PRECIOUS: .print
P= | lp
P_OPTS	= -o-o12 -o-c17

CC	= cc
CFLAGS	= -O
LIB	= -ltermlib
LIB	= -lcurses

BIN_DIR	= /usr/local/bin
MAN_DIR	= /usr/man/local/man1

BIN	= rep
MAN	= $(BIN).1
MAKEFILE= Makefile
CFILES	= $(BIN).c
OFILES	=
OTHER	=
ALL	= $(BIN) $(OFILES)

all: $(BIN)

install: $(BIN_DIR)/$(BIN) $(MAN_DIR)/$(MAN)

$(BIN_DIR)/$(BIN): $(BIN)
	cp $(BIN) $(BIN_DIR)/$(BIN)
	chmod 775 $(BIN_DIR)/$(BIN)
$(MAN_DIR)/$(MAN): $(MAN)
	cp $(MAN) $(MAN_DIR)/$(MAN)

#
#	Housekeeping
#
clean:
	rm -f $(OFILES)

.print: $(MAKEFILE) $(CFILES)
	cp /dev/null print.tmp
	man -d $(MAN) > print.tmp
	pr -f `ls $?` >> print.tmp
	cat print.tmp $P $(PRINTOPTS)
	rm print.tmp
	-touch .print

#
#	specific makes
#
$(BIN): $(BIN).c
	$(CC) $(CFLAGS) $@.c $(LIB) -o $@
