PICDATA = /staff/dhb/data/picdata
PAGER   = -DPAGER=\"/usr/ucb/more\"
GETOPT  =
CFLAGS  = -O -DPICDATA=\"$(PICDATA)\" $(PAGER)
LINT    = -hbxac

all:	calndr pic_m2h pic_h2m

calndr:	calndr.o
	$(CC) $(LFLAGS) calndr.o $(GETOPT)
	mv a.out $@

pic_m2h:	pic_m2h.o
	$(CC) $(LFLAGS) pic_m2h.o
	mv a.out $@

pic_h2m:	pic_h2m.o
	$(CC) $(LFLAGS) pic_h2m.o
	mv a.out $@

lint:	.
	lint $(LINT) calndr.c
	lint $(LINT) pic_m2h.c
	lint $(LINT) pic_h2m.c
