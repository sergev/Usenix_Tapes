#
#	Makefile for genp
#
PROGRAM=	genp
ALL=		$(PROGRAM)
INST=		in.genp in.genp.man

TARGET_DIR=	/usr/local/bin
MAN_DIR=	/usr/man/local/man1

CC=	cc
CFLAGS=	-O -n -s

genp: genp.c
	$(CC) $(CFLAGS) genp.c -o genp
I.genp: genp
	cp genp $(TARGET_DIR)/genp
	chmod +x $(TARGET_DIR)/genp

genp.man: genp.1l
	man -d genp.1l > genp.man
I.genp.man:
	cp genp.1l $(MAN_DIR)/genp.1l

install: $(INST)
clean:
	rm -f $(ALL)
