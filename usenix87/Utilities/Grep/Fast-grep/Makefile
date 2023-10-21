# optional items for ENV:
# -I.			use regexp.h in current directory, not /usr/include.
# -DEGREPOLD=path	location of std egrep (normally /usr/bin/egrep).
# -DGREPOLD=path	location of std grep (normally /bin/grep).
# -DFGREPOLD=path	location of std fgrep (normally /usr/bin/fgrep).
# -Dstrrchr=rindex, -Dstrchr=index	for troglodytes.
# -DSLOWSYS		invoke xread() for system time quirk on PDP, others? 
# -DNOKANJI		default is for Japanese Unix.  undef only for raw
#			 parity-marked search capability, not standard w/grep.
# -DCHINESE		for systems using EUC Chinese2 codes

ENV= -I.

# warning:  do not overwrite existing [ef]?grep family with $BIN path choice
BIN= /usr/local

# optional items for OBJ:
# misc.o		for V7 or BSD 4.2 systems w/no getopt(3) or string(3)
#			  also contains xread() per above.
# regexp.o		if Henry Spencer's regexp(3) is not installed
#			V8 people -- your regexp.h won't do

OBJ= regexp.o

CFLAGS= -O $(ENV) 
#CFLAGS= -O -i $(ENV)	uncomment this line for PDP-11

regexp:	; make -f Makefile.regexp r; make egrep

egrep:	egrep.o regerror.o
	cc $(CFLAGS) egrep.o regerror.o -o egrep $(OBJ)
	rm -f grep fgrep
	ln egrep grep
	ln egrep fgrep

install: 
	rm -f $(BIN)/*grep
	strip egrep
	mv egrep $(BIN)
	ln $(BIN)/egrep $(BIN)/grep
	ln $(BIN)/egrep $(BIN)/fgrep

clean:
	rm *.o ./egrep ./grep ./fgrep ./try
