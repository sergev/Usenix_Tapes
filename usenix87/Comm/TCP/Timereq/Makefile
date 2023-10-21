#
# TCP/IP time client
#

BINDIR = /usr/local/bin

CFLAGS = -O

rdate: rdate.o
	cc rdate.o -o rdate

clean:
	rm -f *.o .,* a.out ERRS rdate

install: all
	install -s rdate $(DESTDIR)/$(BINDIR)
