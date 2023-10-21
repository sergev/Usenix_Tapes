CFLAGS=		-O

OFILES=		ansi.o basic.o bind.o buffer.o crypt.o dg10.o \
		display.o eval.o exec.o file.o fileio.o \
		hp110.o hp150.o ibmpc.o input.o isearch.o line.o \
		lock.o main.o random.o region.o search.o spawn.o \
		st520.o tcap.o termio.o tipc.o vmsvt.o vt52.o \
		window.o word.o z309.o

CFILES=		ansi.c basic.c bind.c buffer.c crypt.c dg10.c \
		display.c eval.c exec.c file.c fileio.c \
		hp110.c hp150.c ibmpc.c input.c isearch.c line.c \
		lock.c main.c random.c region.c search.c spawn.c \
		st520.c tcap.c termio.c tipc.c vmsvt.c vt52.c \
		window.c word.c z309.c

HFILES=		estruct.h edef.h efunc.h epath.h ebind.h evar.h

emacs:		$(OFILES)
		$(CC) $(CFLAGS) $(OFILES) -ltermcap -lc -o emacs

$(OFILES):	$(HFILES)
