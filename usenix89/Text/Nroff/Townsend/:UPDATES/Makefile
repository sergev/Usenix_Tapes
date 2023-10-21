#	Makefile for table.c, method of producing a single terminal
#	table for nroff under USG UNIX.
#	Makefile written by Bruce Townsend (utcs!bnr-vpa!bruce)
#	and Ian Darwin (utcs!darwin!ian)
#	March 6, 1985.

TABFILES = tabXXX
DESTDIR = /usr/lib/term
LINTTAB = tabXXX
DEFS =
CFLAGS = $(DEFS) -O
LDFLAGS = -s
LFLAGS = $(DEFS)

$(TABFILES): dummy_file
	$(MAKE) -$(MAKEFLAGS) table.o $@.o
	cc $(LDFLAGS) table.o $@.o
	./a.out $@

# This dummy_file should never exist, it is a kludge to get Make to
# always execute cc.
dummy_file:
	@rm -f dummy_file

all:	$(TABFILES)

install: $(TABFILES)
	mv $(TABFILES) $(DESTDIR)

lint:	table.c
	lint $(LFLAGS) table.c $(LINTTAB).c

elbat:	elbat.o
	cc $(LDFLAGS) elbat.o -o elbat

clean:
	rm -f a.out tab*.o elbat.o
