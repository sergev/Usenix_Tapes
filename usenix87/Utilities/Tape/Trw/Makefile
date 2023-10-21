
# This is the makefile for the IBM/EBCDIC-ANSI standard tape handler.

OBJECTS = main.o dsrep.o err.o convutil.o readtp.o gutil.o writetp.o
SOURCES = main.c dsrep.c err.c convutil.c readtp.c gutil.c writetp.c
INSDIR	= /usr/local
OWNER	= bin
GROUP	= bin

trw: $(OBJECTS)             
	cc $(OBJECTS) -o trw

install: $(INSDIR)/trw
	:
$(INSDIR)/trw: trw
	mv trw $(INSDIR)/trw
	chmod 755 $(INSDIR)/trw
	chown $(OWNER) $(INSDIR)/trw
	chgrp $(GROUP) $(INSDIR)/trw

clean:
	rm -f $(OBJECTS) core a.out

shar:
	shar -a README trw.1 Makefile *.h dec.orig $(SOURCES) >trw.shar

main.o:     dec.h
dsrep.o:    dec.h
err.o:      dec.h
gutil.o:    dec.h
convutil.o: tables.h
readtp.o:   dec.h
writetp.o:  dec.h
