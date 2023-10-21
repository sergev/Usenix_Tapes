CFLAGS=		-O

OFILES=		ansi.o basic.o bind.o buffer.o display.o file.o \
		fileio.o hp150.o line.o lock.c main.o random.o region.o \
		search.o spawn.o tcap.o termio.o vt52.o window.o word.o \
		exec.o eval.o isearch.o input.o

CFILES=		ansi.c basic.c bind.c buffer.c display.c  file.c \
		fileio.c hp150.c line.c lock.c main.c random.c region.c \
		search.c spawn.c tcap.c termio.c vt52.c window.c word.c \
		exec.c eval.c isearch.c input.c

HFILES=		estruct.h edef.h efunc.h epath.h ebind.h evar.h esearch.h

emacs:		$(OFILES)
		$(CC) $(CFLAGS) $(OFILES) -lcurses -lc -o emacs

$(OFILES):	$(HFILES)
