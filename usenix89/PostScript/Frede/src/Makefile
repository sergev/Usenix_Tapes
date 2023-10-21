CD=builtin cd
all: tpscript.d others
MFLAGS=SYS=$(SYS) VERBOSE=$(VERBOSE)

tpscript.d:
	$(CD) tpscript; $(MAKE) $(MFLAGS)

others:
	$(CD) opscript; $(MAKE) $(MFLAGS)
