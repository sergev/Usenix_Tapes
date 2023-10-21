TARGET=timer.a
CFLAGS=-O
LINT=lint -uhbaxpc
CTAGS=ctags
PRINT=@pr -t

CFILES=\
	timer.c
HFILES=\
	timer.h\
	/usr/include/sys/timeb.h\
	/usr/include/sys/types.h
OBJECTS=\
	timer.o

.SUFFIXES: .i

$(TARGET):	$(OBJECTS)
	ar rv $@ $?
	ranlib $@

all:	$(TARGET) timer.man
	
lint:
	$(LINT) $(PREFLAGS) $(CFILES) -lc

tags:	$(HFILES) $(CFILES)
	$(CTAGS) $(HFILES) $(CFILES)

print:
	$(PRINT) $(HFILES) $(FFILES) $(CFILES)

.c.o:
	$(CC) $(CFLAGS) -c $<

.c.i:
	$(CC) $(CFLAGS) -P $<

.c.s:
	$(CC) $(CFLAGS) -S $<

timer.o:\
	/usr/include/sys/types.h\
	/usr/include/sys/timeb.h\
	timer.h

timer.man:
	nroff -man timer.3 > timer.man
