# Quickie makefile
CFLAGS	= -C
printf:		printf.c
	$(CC) $(CFLAGS) -o printf printf.c
# Edit appropriately
DESTPROG=/usr/local/bin/printf
DESTMAN =/usr/man/man1/printf.1
install:	printf
	cp printf $(DESTPROG)
	chmod 755 $(DESTPROG)
	strip $(DESTPROG)
	cp printf.1 $(DESTMAN)
	chmod 444 $(DESTMAN)
