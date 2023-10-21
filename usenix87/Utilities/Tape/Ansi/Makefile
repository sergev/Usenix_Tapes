# A system for handling magtapes, real or in tape image form.
# Copyright Dick Grune
# make all:	make all the shell commands
# make install:	install all the shell commands
# make crout:	install C-routine library
# make NOS:	install NOSsplit et al. for reading Cyber tapes
# make man:	install the manuals
# make lint:	`lint' everything
# make clean:	clean up *.o
#

USR=/user1/dick
BIN=$(USR)/bin
LIB=$(USR)/lib
INC=$(USR)/src
MAN1=/user1/dick/man
MAN3=/user1/dick/man
OWN=dick
GRP=staff

CFLAGS=-O -Dindex=strchr

all: survey rawtp cptp ansir ansiw NOSsplit NOStr

install: survey rawtp cptp ansir ansiw
	mv survey $(BIN)/survey
	- /etc/chown $(OWN) $(BIN)/survey
	- /etc/chgrp $(GRP) $(BIN)/survey
	mv rawtp $(BIN)/rawtp
	- /etc/chown $(OWN) $(BIN)/rawtp
	- /etc/chgrp $(GRP) $(BIN)/rawtp
	mv cptp $(BIN)/cptp
	- /etc/chown $(OWN) $(BIN)/cptp
	- /etc/chgrp $(GRP) $(BIN)/cptp
	mv ansir $(BIN)/ansir
	- /etc/chown $(OWN) $(BIN)/ansir
	- /etc/chgrp $(GRP) $(BIN)/ansir
	mv ansiw $(BIN)/ansiw
	- /etc/chown $(OWN) $(BIN)/ansiw
	- /etc/chgrp $(GRP) $(BIN)/ansiw

crout:	tp.h libt.a
	cp tp.h $(INC)/tp.h
	- /etc/chown $(OWN) $(INC)/tp.h
	- /etc/chgrp $(GRP) $(INC)/tp.h
	mv libt.a $(LIB)/libt.a
	- /etc/chown $(OWN) $(LIB)/libt.a
	- /etc/chgrp $(GRP) $(LIB)/libt.a

NOS:	NOSsplit NOStr
	mv NOSsplit $(BIN)/NOSsplit
	- /etc/chown $(OWN) $(BIN)/NOSsplit
	- /etc/chgrp $(GRP) $(BIN)/NOSsplit
	mv NOStr $(BIN)/NOStr
	- /etc/chown $(OWN) $(BIN)/NOStr
	- /etc/chgrp $(GRP) $(BIN)/NOStr

ansir:	ansir.o libt.a
	$(CC) $(CFLAGS) ansir.o libt.a -o ansir

ansiw:	ansiw.o libt.a
	$(CC) $(CFLAGS) ansiw.o libt.a -o ansiw

ansir.o ansiw.o:	ansi.h

cptp:	cptp.o libt.a
	$(CC) $(CFLAGS) cptp.o libt.a -o cptp

rawtp:	rawtp.o libt.a
	$(CC) $(CFLAGS) rawtp.o libt.a -o rawtp

survey:	survey.o libt.a
	$(CC) $(CFLAGS) survey.o libt.a -o survey

survey.o rawtp.o cptp.o ansir.o ansiw.o:	tp.h tploc.h

NOSsplit:	NOSsplit.o libt.a
	$(CC) $(CFLAGS) NOSsplit.o libt.a -o NOSsplit

NOSsplit.o:	tp.h tploc.h

NOStr:		NOStr.o
	$(CC) $(CFLAGS) NOStr.o -o NOStr

NOStr.o:	tp.h

OBJ=tpread.o tpwrite.o tpopen.o tpclose.o tpname.o tpwtmloc.o \
		etoa.o tprdloc.o tpwloc.o

$(OBJ):	tp.h tploc.h

libt.a:	$(OBJ)
	ar cr libt.a `lorder $(OBJ) | tsort`
#	ar cr libt.a $(OBJ)
#	ranlib libt.a

man:
	cp NOSsplit.1 $(MAN1)/NOSsplit.1
	- /etc/chown $(OWN) $(MAN1)/NOSsplit.1
	- /etc/chgrp $(GRP) $(MAN1)/NOSsplit.1
	cp NOStr.1 $(MAN1)/NOStr.1
	- /etc/chown $(OWN) $(MAN1)/NOStr.1
	- /etc/chgrp $(GRP) $(MAN1)/NOStr.1
	cp ansir.1 $(MAN1)/ansir.1
	- /etc/chown $(OWN) $(MAN1)/ansir.1
	- /etc/chgrp $(GRP) $(MAN1)/ansir.1
	cp ansiw.1 $(MAN1)/ansiw.1
	- /etc/chown $(OWN) $(MAN1)/ansiw.1
	- /etc/chgrp $(GRP) $(MAN1)/ansiw.1
	cp cptp.1 $(MAN1)/cptp.1
	- /etc/chown $(OWN) $(MAN1)/cptp.1
	- /etc/chgrp $(GRP) $(MAN1)/cptp.1
	cp mag.1 $(MAN1)/mag.1
	- /etc/chown $(OWN) $(MAN1)/mag.1
	- /etc/chgrp $(GRP) $(MAN1)/mag.1
	cp mag.3 $(MAN3)/mag.3
	- /etc/chown $(OWN) $(MAN1)/mag.3
	- /etc/chgrp $(GRP) $(MAN1)/mag.3
	cp rawtp.1 $(MAN1)/rawtp.1
	- /etc/chown $(OWN) $(MAN1)/rawtp.1
	- /etc/chgrp $(GRP) $(MAN1)/rawtp.1
	cp survey.1 $(MAN1)/survey.1
	- /etc/chown $(OWN) $(MAN1)/survey.1
	- /etc/chgrp $(GRP) $(MAN1)/survey.1

lint:
	lint survey.c llib-lmag
	lint rawtp.c llib-lmag
	lint cptp.c llib-lmag
	lint ansir.c llib-lmag
	lint ansiw.c llib-lmag
	lint NOSsplit.c llib-lmag
	lint NOStr.c llib-lmag

clean:
	rm *.o
