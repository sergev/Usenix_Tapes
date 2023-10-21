OBJS = uustatd.o lookdir.o

# CHANGE THIS FOR YOUR LOCAL SITE
# YOU MUST ALSO CHANGE the include file config.h for your local site.
# All that matters for lookdir.c is whether you have
#	DIRFIXLEN - USG 14 char fixed length file names
#	DIRVARLEN - 4.X BSD variable (up to 255 chars) per directory entry
#
INSTALLDIR = .

uustatd: $(OBJS)
	cc $(LDFLAGS) $(OBJS)
	mv a.out $@

lookdir.o: machine.h config.h

uustatd.o:

# install uustat with set uid uucp so it has permission read
# the uucp spool files.  You must do the install as root.
install: uustatd
	chown uucp uustatd
	chmod 4751 uustatd
	-mv uustatd $(INSTALLDIR)

clean:
	rm -f $(OBJS) uustatd a.out core
