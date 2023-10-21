# where to install executable on 'make install'
DEST=/usr/local/bin/gnuplot
# where to install man page on 'make man_install'
MANDEST=/usr/man/manl/gnuplot.l

# -DFORK if you have vfork()
# -DBCOPY if your memcpy() is called bcopy() (a Berkeleyism, right?)
# -DNOCOPY if you don't have a memcpy() by any name
# -DGAMMA if you've got gamma(3)
# -O if you trust your compiler's optimizer
CFLAGS = -DVFORK -DBCOPY -DGAMMA #-gx #-O

# -D<terminal> in TERMFLAGS iff you wish to support <terminal>
# -DAED         AED 512 and AED 767
# -DBITGRAPH    BBN BitGraph
# -DHP26        HP2623A and maybe others
# -DHP75        HP7580, and probably other HPs
# -DPOSTSCRIPT  Postscript
# -DQMS         QMS/QUIC laserprinter (Talaris 1200 and others)
# -DREGIS       ReGis graphics (vt125, vt220, vt240, Gigis...)
# -DSELANAR     Selanar
# -DTEK         Tektronix 4010, and probably others
# -DUNIXPLOT    unixplot
# -DV384        Vectrix 384 and tandy color printer

TERMFLAGS = -DAED -DBITGRAPH -DHP26 -DHP75 -DPOSTSCRIPT -DQMS -DREGIS -DSELANAR -DTEK -DUNIXPLOT -DV384


OBJS = 	command.o eval.o graphics.o internal.o misc.o parse.o plot.o scanner.o\
    standard.o term.o util.o

CSOURCE1 = command.c eval.c graphics.c internal.c misc.c parse.c
CSOURCE2 = plot.c scanner.c standard.c term.c util.c version.c
# not C code, but still needed
ETC = README gnuplot.1 Makefile plot.h simple.demo 1.dat 2.dat 3.dat \
	  controls.demo
# VMS and PC-specific files
VMSPC = compile.com link.com \
        buildhlp.c corgraph.asm corplot.c header.mac help.c help.hlm \
		hrcgraph.asm lineproc.mac link.opt make.msc pcgraph.asm

# -lplot if you have -DUNIXPLOT
LIBS = -lm -lplot

gnuplot: $(OBJS) version.o
	cc $(CFLAGS) $(OBJS) version.o $(LIBS) -o gnuplot

install: gnuplot
	cp gnuplot $(DEST)
	strip $(DEST)

man_install: gnuplot.1
	cp gnuplot.1 $(MANDEST)

term.o: term.c 
	cc $(CFLAGS) $(TERMFLAGS) -c term.c

$(OBJS): plot.h

lint:
	lint -hx $(CSOURCE1) $(CSOURCE2)

vax:
	uucp -m $(CSOURCE1) $(CSOURCE2) plot.h excalibur!m1:\[esl.source.esl.gnuplot\]

shar: gnuplot.shar.1 gnuplot.shar.2 gnuplot.shar.3 gnuplot.shar.4 \
	  gnuplot.shar.5 gnuplot.shar.6

gnuplot.shar.1: $(ETC)
	shar -vc $(ETC) > gnuplot.shar.1

gnuplot.shar.2: $(CSOURCE1)
	shar -vc $(CSOURCE1) > gnuplot.shar.2

gnuplot.shar.3: $(CSOURCE2)
	shar -vc $(CSOURCE2) > gnuplot.shar.3

gnuplot.shar.4: docs/*
	cd docs; make clean
	shar -vc docs > gnuplot.shar.4

gnuplot.shar.5: /usr/help/gnuplot/*
	(cd /usr; shar -vc help/gnuplot) > gnuplot.shar.5

gnuplot.shar.6: $(VMSPC)
	shar -vc $(VMSPC) > gnuplot.shar.6
