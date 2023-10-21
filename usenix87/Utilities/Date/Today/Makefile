#
#	Makefile for very verbose date command (today).
#
.PRECIOUS: .print
P= | lp
PRINTOPTS = -o-o7 -o-c12

CC	= cc
CFLAGS	= -O -n -s

BIN_DIR	= /usr/local/bin
MAN_DIR	= /usr/man/local/man1

PROGRAM	= today
MAN	= $(PROGRAM).1l
MAKEFILE= Makefile
CFILES	= datetx.c moontx.c nbrtxt.c timetx.c today.c
OFILES	= datetx.o moontx.o nbrtxt.o timetx.o today.o
OTHER	=

all:	$(PROGRAM)

install: $(BIN_DIR)/$(PROGRAM) $(MANDIR)/$(MAN)
$(BIN_DIR)/$(PROGRAM): $(PROGRAM)
	cp $(PROGRAM) $(BIN_DIR)/$(PROGRAM) ; chmod 775 $(BIN_DIR)/$(PROGRAM)
$(MAN_DIR)/$(MAN): $(MAN)
	cp $(MAN) $(MAN_DIR)/$(MAN)

clean:
	rm -f $(OFILES)

.print:	$(MAKEFILE) $(CFILES)
	man -d $(MAN) > tmp
	pr -f `ls $?` >> tmp
	cat tmp $P $(PRINTOPTS)
	rm tmp
	-touch .print

today:	$(OFILES)
	$(CC) -o $@ $(OFILES)
