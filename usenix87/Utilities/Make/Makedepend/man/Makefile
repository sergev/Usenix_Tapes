all: makedep.man makenorm.man depend.man

ROFF= nroff

.1.man:
	$(ROFF) -man $< > $@

.SUFFIXES: .man .1
