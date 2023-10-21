SRCS= paintps.c
OBJS= paintps.o
COMMANDS= paintps
CFLAGS= -O
BIN= /usr/local
MAN= /usr/man/manl

SPOOLCMD= "maclaser"

all: $(COMMANDS)

paintps: paintps.c
	$(CC) $(CFLAGS) paintps.c -o paintps '-DSPOOLCMD=$(SPOOLCMD)'

install: $(COMMANDS)
	install -s -m 0775 paintps ${DESTDIR}/${BIN}
	install -c -m 0444 paintps.1 ${DESTDIR}/${MAN}

clean:
	rm -f *.o a.out core $(COMMANDS)
