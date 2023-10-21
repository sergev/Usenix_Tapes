#
#	Makefile for $(BIN)
#
.PRECIOUS: .print
P= | lp
P_OPTS	= -o-o12 -o-c17

CC	= cc
CFLAGS	= -O -n -s -Dindex=strchr
#-DMONITOR="file"
#				saves monitor info in file (local only)
#-DCATMANDIR="dir"
#				looks for manual entries here

BIN_DIR	= /usr/local/bin
MAN_DIR	= /usr/man/local/man1

BIN	= pman
MAN	= $(BIN).1
MAKEFILE= Makefile
CFILES	= getopt.c pman.c
OFILES	= getopt.o pman.o
OTHER	= :Author pman.nr pman.t
ALL	= $(BIN) $(OFILES)

all: $(BIN)

install: $(BIN_DIR)/$(BIN) $(MANDIR)/$(MAN)

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
	man -d $(MAN) > tmp
	pr -f `ls $?` >> tmp
	cat tmp $P $(PRINTOPTS)
	rm tmp
	-touch .print

#
#	specific makes
#
$(BIN): $(MAKEFILE) $(OFILES)
	$(CC) $(CFLAGS) -o $@ $(OFILES)
