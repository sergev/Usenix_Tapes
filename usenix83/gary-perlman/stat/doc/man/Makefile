MACROS = -man
PIPE = | npr
FORMAT = nroff
OPTIONS = -T450-12 -e
DUMMY = /dev/null
all: $(DUMMY)
	make pair
	make pof
	make regress
	make transpose
	make validata
	make vincent
.PRECIOUS: CALC DM abut anova biplot calc critf desc dm dprime io pair pof regress transpose validata vincent
make CALC: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	-me	CALC	$(PIPE)
DM: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	-me	DM	$(PIPE)
abut: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	abut	$(PIPE)
anova: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	anova	$(PIPE)
biplot: $(DUMMY)
	 make pair
calc: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	calc	$(PIPE)
corr: $(DUMMY)
	make regress
critf: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	critf	$(PIPE)
desc: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	desc	$(PIPE)
dm: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	dm	$(PIPE)
dprime: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	dprime	$(PIPE)
io: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	io	$(PIPE)
pair: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	pair	$(PIPE)
pof: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	pof	$(PIPE)
regress: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	regress	$(PIPE)
transpose: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	transpose	$(PIPE)
validata: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	validata	$(PIPE)
vincent: $(DUMMY)
	$(FORMAT)	$(OPTIONS)	$(MACROS)	vincent	$(PIPE)
