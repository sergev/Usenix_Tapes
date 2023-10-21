# Makefile for 4.1BSD

CFLAGS=	-DVMUNIX -DBSD41 -O
LIBES=	-ljobs
SRCS=	spool.c despool.c header.c readdev.c
HDRS=	spool.h

all: spool header despool

install: inst-spool inst-header inst-despool

inst-spool:
	mv spool /usr/bin
	/etc/chown root /usr/bin/spool
	chmod 4755 /usr/bin/spool

inst-header:
	mv header /usr/lib/std_header

inst-despool:
	mv despool /usr/lib
	/etc/chown root /usr/lib/despool
	chmod 4755 /usr/lib/despool

spool:	spool.o readdev.o
	cc -o spool spool.o readdev.o $(LIBES)

header:	header.o readdev.o
	cc -o header header.o readdev.o

despool: despool.o readdev.o
	cc -o despool despool.o readdev.o -lndir

readdev.o: readdev.c spool.h
spool.o: spool.c spool.h
header.o: header.c spool.h
despool.o: despool.c spool.h

# the "script" entry is purely for making distributions
script: $(SRCS) $(HDRS)
	makescript /tmp/spoolscript $(SRCS) $(HDRS) spool.1l spool.5l\
		spool_devices Makefile
