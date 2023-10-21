MAKE=make
LIBDIR=./_unix.O
BINDIR=./BIN
EMPGLB=$(LIBDIR)/empglb.o
EMPDIS=$(LIBDIR)/empdis.o
EMPSUB=$(LIBDIR)/empsub.a
EMPCOM=$(LIBDIR)/empcom.a
EMPS=\
	$(BINDIR)/emp1 \
	$(BINDIR)/emp2 \
	$(BINDIR)/emp3 \
	$(BINDIR)/emp4 \
	$(BINDIR)/emp5 \
	$(BINDIR)/emp7 \
	empn

all:	$(EMPGLB) $(EMPDIS) $(EMPSUB) $(EMPCOM) \
	empire empcre \
	$(BINDIR)/empfix empn
	@echo All done!

$(EMPGLB):	FRC
	@echo EMPGLB:
	@cd EMPGLB; $(MAKE)

empglb.o:	$(EMPGLB)

$(EMPDIS):	FRC
	@echo EMPDIS:
	@cd EMPDIS; $(MAKE)

empdis.o:	$(EMPDIS)

$(EMPSUB):	FRC
	@echo EMPSUBS:
	@cd EMPSUBS; $(MAKE)

empsub.a:	$(EMPSUB)

$(EMPCOM):	FRC
	@echo COMMANDS:
	@cd COMMANDS; $(MAKE)

empcom.a:	$(EMPCOM)

empire:		FRC
	@echo EMPIRE:
	@cd EMPIRE; $(MAKE)

empcre:		FRC
	@echo EMPCRE:
	@cd EMPCRE; $(MAKE)

$(BINDIR)/empfix:	empfix

empfix:		FRC
	@echo EMPFIX:
	@cd EMPFIX; $(MAKE)

$(EMPS):	FRC
	@echo EMPN: $(@F);		# 4.2 change $(@F) to $@
	@cd EMPN; $(MAKE) $(@F);	# 4.2 change $(@F) to $@

emp1:	$(BINDIR)/emp1

emp2:	$(BINDIR)/emp2

emp3:	$(BINDIR)/emp3

emp4:	$(BINDIR)/emp4

emp5:	$(BINDIR)/emp5

emp7:	$(BINDIR)/emp7

FRC:
