PLSTARTUP="Startup"
BIN="gprolog1.5"
GRIND=igrind

# Replace VAX by IEEE for IEEE floating point machines (e.g. Sun)
FLOATING=IEEE

# Replace the right-end side by the empty string to get
# -1 as end of file character

EOF=

CFLAGS=-w -O $(EOF) -D$(FLOATING) -DGRAPHICS -DSTARTUPFILE=\"$(PLSTARTUP)\"
# change define in parms.c - no mistakes then!
OBJECTS=main.o unify.o rewrite.o dbase.o sysbits.o space.o trace.o\
	parms.o arith.o compare.o auxfn.o gr1.o gr2.o gr3.o prtable.o\
	pushargs.o

CProlog : $(OBJECTS)
	$(CC) -o CProlog -s $(OBJECTS) -lg -ltermcap -lm

main.o : arithop.h evalp.h

gr1.o   : gr.h
	$(CC) $(CFLAGS) -c gr1.c
	ld -r gr1.o -lcore -lsunwindow -lpixrect
	mv a.out gr1.o
gr2.o   : gr.h
gr3.o   : gr.h

(OBJECTS) : pl.h
arith.o : arithop.h

pushargs.o : pushargs.s
	as -o pushargs.o pushargs.s

startup : CProlog pl/init.pl
	./CProlog -b pl/init.pl <bootcmd

install : CProlog startup
	mv startup $(PLSTARTUP)
	mv CProlog $(BIN)

doc :
	nroff gprolog.nr > gprolog.doc

grind:
	$(GRIND) pl.h evalp.h arithop.h main.c rewrite.c dbase.c auxfn.c \
		arith.c unify.c compare.c sysbits.c space.c parms.c
	$(GRIND) -x index
