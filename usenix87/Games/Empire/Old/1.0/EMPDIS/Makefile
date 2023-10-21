CFLAGS=-c
HDRDIR=../hdrs
INSDIR=../_unix.O
EMPDIS=$(INSDIR)/empdis.o

$(EMPDIS):	$(HDRDIR)/empdef.h  empdis.c
	$(CC) $(CFLAGS) -I$(HDRDIR) empdis.c
	mv empdis.o $@

clobber:
	rm -f $(EMPDIS)
