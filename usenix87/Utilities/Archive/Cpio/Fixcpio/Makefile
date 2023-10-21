fixcpio:	fixcpio.c
	$(CC) $(CFLAGS) -o fixcpio fixcpio.c

# Edit appropriately.
DESTDIR	= /usr/local/bin
MANDIR	= /usr/man/man1
MANPAGE	= fixcpio.1
install:	fixcpio
	cp fixcpio $(DESTDIR)
	strip $(DESTDIR)/fixcpio
	cp fixcpio.1 $(MANDIR)/$(MANPAGE)
