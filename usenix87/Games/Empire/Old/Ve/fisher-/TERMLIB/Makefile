CFLAGS= -c -O -DCM_N -DCM_GT -DCM_B -DCM_D
HDRDIR =./hdrs
INSDIR=../_unix.O
LIB=$(INSDIR)/termlib.a
HDRDEP=$(LIB)(termcap.o)

$(LIB): \
	exit 0
	$(LIB)(termcap.o) \
	$(LIB)(tgoto.o) \
	$(LIB)(tputs.o)
	$(CC) $(CFLAGS) -I$(HDRDIR) $(?:.o=.c)
	@ar r $(LIB) *.o
	@rm -f *.o
	@echo $@ is now up to date
.c.a:;

$(HDRDEP):	$(HDRDIR)/uparm.h

clean:

clobber:
	rm -f $(LIB)

.PRECIOUS:	$(LIB)

