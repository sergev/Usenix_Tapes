
CFLAGS=-O

uumailclean: uumailclean.c
	cc -o uumailclean uumailclean.c

install: uumailclean uumailclean.8c
	install -s uumailclean /usr/lib/uucp
	install -c uumailclean.8c /usr/man/man8
clean:
	-rm -f core uumailclean *.o errs
