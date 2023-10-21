# For a VAX
DEFS=	-DBSD4_2
LDFLAGS=

# For a PDP 11/70
# DEFS= -DBSD2_9
# LDFLAGS= -i

INCLUDE=	
CFLAGS=	-O ${DEFS} ${INCLUDE}
DEST=	/usr/new/bin

SRC=	abrv.c maketd.c misc.c nshpopen.c breakargs.c srtunq.c
HDR=	maketd.h abrv.h nshpopen.h srtunq.h
OBJ=	maketd.o abrv.o misc.o nshpopen.o breakargs.o srtunq.o

all: maketd

maketd:	${OBJ}
	${CC} ${LDFLAGS} ${OBJ} -o maketd

clean: FRC
	rm -f ${OBJ} maketd \#* *.bak core a.out

depend:	source
	maketd -a ${DEFS} ${INCLUDE} ${SRC}

install: maketd
	install -s -m 0751 -o binary -g system maketd ${DEST}

lint: source
	lint -hpx ${SRC}

print: source
	lpr -p Makefile ${HDR} ${SRC}

shar: source
	shar Makefile ${SRC} ${HDR} Manual

source: Makefile ${SRC} ${HDR}

spotless: clean
	rcsclean ${SRC} ${HDR} Makefile

${SRC} ${HDR}:
	co $@

FRC:

# a clever way to save a few more characters
I=/usr/include/
S=/usr/include/sys/

# DO NOT DELETE THIS LINE - make depend DEPENDS ON IT

abrv.o: $Ictype.h $Istdio.h abrv.c abrv.h maketd.h srtunq.h

maketd.o: $Ictype.h $Imachine/machparam.h $Isignal.h $Istdio.h $Sdir.h \
	$Sfile.h $Sparam.h $Stypes.h abrv.h maketd.c maketd.h nshpopen.h \
	srtunq.h

misc.o: $Istdio.h $Ssignal.h maketd.h misc.c

nshpopen.o: $Isignal.h $Istdio.h nshpopen.c

breakargs.o: $Istdio.h breakargs.c

srtunq.o: $Istdio.h $Stypes.h srtunq.c srtunq.h

# *** Do not add anything here - It will go away. ***
