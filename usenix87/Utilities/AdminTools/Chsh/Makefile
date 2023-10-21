CFLAGS=-O
DEST=/usr/local
DOC_DEST=/usr/man/man9

chfn chsh: chsh.o
	cc $(CFLAGS) chsh.o -o chsh
	-rm chfn
	ln chsh chfn

install: chsh chfn
	@echo Warning: you must be superuser to do this.
	-rm $(DEST)/chsh $(DEST)/chfn
	cp chsh $(DEST)/chsh
	ln $(DEST)/chsh $(DEST)/chfn
	chown root $(DEST)/chsh
	chmod 4711 $(DEST)/chsh
	cp chsh.1 $(DOC_DEST)/chsh.1
	cp chfn.1 $(DOC_DEST)/chfn.1
