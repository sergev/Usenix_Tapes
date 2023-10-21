CFLAGS = -O -DHPUX5
DESTDIR = /usr/local/bin
DOCDIR = /usr/man/man1

untic:	untic.c
	$(CC) $(CFLAGS) untic.c -o untic

lint:
	lint untic.c

install:	untic
	cp untic $(DESTDIR)/untic
	strip $(DESTDIR)/untic
	cp untic.1 $(DOCDIR)/untic.1

clean:
	rm untic
