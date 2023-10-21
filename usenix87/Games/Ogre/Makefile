#
#	Makefile for $(BIN)
#
.PRECIOUS: .print
P= | lp
P_OPTS	= -o-o12 -o-c17

CC	= cc
CFLAGS	= -O -n -s -DsysV

BIN_DIR	= /usr/games
MAN_DIR	= /usr/man/local/man6


BIN	= ogre
MAN	= $(BIN).6
MAKEFILE= Makefile
CFILES	= attack.c init.c initround.c main.c map.c move.c ogrecom.c \
	  ogrestat.c resolve.c termcap.c
OFILES	= attack.o init.o initround.o main.o map.o move.o ogrecom.o \
	  ogrestat.o resolve.o termcap.o
HFILES	= ext.h ogre.h
OTHER	= AUTHOR README ogres
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
$(BIN): $(OFILES)
	$(CC) -o $@ $(OFILES)
