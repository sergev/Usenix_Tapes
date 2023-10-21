CC=cc
SHELL=/bin/sh
CFLAGS= -O -DVSH -DSTOPABLE -DVT100ARROW
LDFLAGS= -g
LOCALLIB=strlib.a
LIBS=
BIN=/usr/local/bin
DESTDIR=/usr/lib/vsh
OBJECTS= main.o at.o dir.o xeq.o curdir.o process.o enterf.o \
	help.o page.o dirlist.o tty.o remove.o file.o show.o \
	ascii.o make.o grep.o showopen.o strings.o \
	curses.o account.o cmdrun.o misccmd.o dircmd.o cmdini.o \
	readarg.o cmdload.o classify.o options.o xecute.o find.o system.o

.c.o:
	$(CC) $(CFLAGS) -c $<

vsh:	$(OBJECTS) mydir.h hd.h $(LOCALLIB)
	cc -n -o nvsh $(LDFLAGS) $(OBJECTS) $(LOCALLIB) -ltermlib $(LIBS)
	-mv vsh ovsh
	mv nvsh vsh
	-rm -f ovsh
	size vsh

install: vsh
	mv $(BIN)/vsh /usr/tmp/vsh
	cp vsh $(BIN)/vsh
	chmod 755 $(BIN)/vsh
	-rm -f /usr/tmp/vsh
	echo 'Do make docs to install documentation'

docs:
	-mkdir $(DESTDIR)
	cp vshrc.gen ../doc/vshelp ../doc/genhelp* ../doc/rmhelp ../doc/showhelp $(DESTDIR)
	echo 'Look over ../doc/vshrc* and ../doc/genhelp.dan to see a sample of'
	echo 'how to configure VSH for a nicer interface. See ../termcap for'
	echo 'additions to your /etc/termcap for a better display.'
	echo 'A manual page for VSH can be found in ../doc .'

strlib.a:
	(cd ../strlib; $(CC) $(CFLAGS) -c str*.c)
	ar rv $(LOCALLIB) ../strlib/str*.o
	-ranlib $(LOCALLIB)

lint:
	lint -bxac *.c

oldinstall: vsh
	vshrc.gen
	cp vsh $(BIN)
	cp genhelp $(DESTDIR)
	cp rmhelp $(DESTDIR)
	cp showhelp $(DESTDIR)
	cp dflt.vshrc $(DESTDIR)

clean:
	rm -f $(OBJECTS) vsh
