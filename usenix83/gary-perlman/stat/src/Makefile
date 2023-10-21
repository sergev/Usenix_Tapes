#	FLAG is passed to the cp command, often it is -i to be safe
CFLAGS = -O
FLAG = -i
#	LIBES determines the libraries to append to the cc command
#	On newer systems, -lm is common, on older systems, -lS (ask a guru)
LIBES = -lm
#	BIN determines to where the final binary file is copied
BIN = /b3/gsp/bin
#	The UNTESTED programs have not been rigorously tested
UNTESTED = echo "This program has not been rigorously tested"
maketrix: maketrix.o eprintf.o checkio.o number.o
	cc maketrix.o eprintf.o checkio.o number.o
	/bin/cp $(FLAG) a.out $(BIN)/maketrix
transpose: transpose.c eprintf.o checkio.o strings.o number.o
	cc transpose.c eprintf.o checkio.o strings.o number.o
	/bin/cp $(FLAG) a.out $(BIN)/transpose
io: io.o checkio.o
	cc io.o checkio.o
	/bin/cp $(FLAG) a.out $(BIN)/io
abut: abut.c eprintf.o number.o
	cc abut.c eprintf.o number.o
	/bin/cp $(FLAG) a.out $(BIN)/abut
anova: anova.c strings.o number.o pof.o checkio.o
	cc anova.c strings.o number.o pof.o checkio.o $(LIBES)
	/bin/cp $(FLAG) a.out $(BIN)/anova
regress: regress.c strings.o number.o pof.o checkio.o
	cc regress.c strings.o number.o pof.o checkio.o $(LIBES)
	/bin/cp $(FLAG) a.out $(BIN)/regress
corr:
	make regress
	/bin/ln $(BIN)/regress $(BIN)/corr
dm: dm.y strings.o number.o eprintf.o checkio.o
	yacc dm.y
	echo "Don't be bothered by shift/reduce conflicts"
	cc y.tab.c number.o strings.o eprintf.o checkio.o $(LIBES)
	/bin/cp $(FLAG) a.out $(BIN)/dm
desc: desc.c pof.o number.o checkio.o
	cc desc.c number.o pof.o checkio.o $(LIBES)
	/bin/cp $(FLAG) a.out $(BIN)/desc
vincent: vincent.c eprintf.o
	cc vincent.c eprintf.o $(LIBES)
	/bin/cp $(FLAG) a.out $(BIN)/vincent
calc: calc.y
	yacc calc.y
	cc y.tab.c $(LIBES)
	/bin/cp $(FLAG) a.out $(BIN)/calc
critf: CRITF.c pof.o critf.o eprintf.o
	cc CRITF.c critf.o pof.o eprintf.o $(LIBES)
	/bin/cp $(FLAG) a.out $(BIN)/critf
pof: POF.c pof.o eprintf.o
	cc POF.c pof.o eprintf.o $(LIBES)
	/bin/cp $(FLAG) a.out $(BIN)/pof
dprime: dprime.c ptoz.o strings.o eprintf.o checkio.o
	cc dprime.c ptoz.o strings.o eprintf.o checkio.o $(LIBES)
	/bin/cp $(FLAG) a.out $(BIN)/dprimenot been seriously tested
	$(UNTESTED)
pair: pair.o number.o pof.o strings.o eprintf.o checkio.o
	cc pair.o number.o pof.o strings.o eprintf.o checkio.o $(LIBES)
	/bin/cp $(FLAG) a.out $(BIN)/pair
biplot:
	make pair
	/bin/ln $(BIN)/pair $(BIN)/biplot
validata: validata.c strings.o checkio.o
	cc validata.c strings.o checkio.o
	/bin/cp $(FLAG) a.out $(BIN)/validata
lsd: lsd.c critf.o pof.o
	cc lsd.c critf.o pof.o -lm
	/bin/cp $(FLAG) a.out $(BIN)/lsd
	$(UNTESTED)
datum: datum.c eprintf.o number.o checkio.o
	cc datum.c eprintf.o number.o checkio.o -lm
	/bin/cp $(FLAG) a.out $(BIN)/datum
