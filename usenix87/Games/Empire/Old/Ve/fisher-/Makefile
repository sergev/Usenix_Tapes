MAKE=make
LIBDIR=./_unix.O
GENLIB=$(LIBDIR)/genlib.a
UPDLIB=$(LIBDIR)/updlib.a

all:	$(GENLIB) $(UPDLIB) eupd ehelp ecre pworld rdship
	@echo All done!

$(GENLIB):	FRC
	@echo GENLIB:
	@cd GENLIB; $(MAKE)

genlib.a:	$(GENLIB)

$(UPDLIB):	FRC
	@echo UPDLIB:
	@cd UPDLIB; $(MAKE)

updlib.a:	$(UPDLIB)

eupd:		FRC
	@echo EUPD:
	@cd EUPD; $(MAKE)

ehelp:		FRC
	@echo EHELP:
	@cd EHELP; $(MAKE)

ecre:		FRC
	@echo ECRE:
	@cd ECRE; $(MAKE)

pworld:		FRC
	@echo "MISC (pworld):"
	@cd MISC; $(MAKE) pworld

rdship:		FRC
	@echo "MISC (rdship):"
	@cd MISC; $(MAKE) rdship

clean:
	@cd GENLIB; $(MAKE) clean
	@cd TERMLIB; $(MAKE) clean
	@cd UPDLIB; $(MAKE) clean
	@cd EUPD; $(MAKE) clean
	@cd EHELP; $(MAKE) clean
	@cd ECRE; $(MAKE) clean
	@cd MISC; $(MAKE) clean

clobber:
	@cd GENLIB; $(MAKE) clobber
	@cd TERMLIB; $(MAKE) clobber
	@cd UPDLIB; $(MAKE) clobber
	@cd EUPD; $(MAKE) clobber
	@cd EHELP; $(MAKE) clobber
	@cd ECRE; $(MAKE) clobber
	@cd MISC; $(MAKE) clobber

.PRECIOUS:	$(GENLIB) $(TERMLIB) $(UPDLIB)

FRC:
