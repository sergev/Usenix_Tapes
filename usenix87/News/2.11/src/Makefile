# '@(#)Makefile.dst	1.23	3/23/87'
# Generic Makefile.
# This is converted to USG/v7/etc by localize.sh
# which should at least be a copy of localize.v7 or localize.usg

# definitions

OSTYPE = usg

# HOME is the user name whose home dir has all the news stuff in it.
HOME=	exptools
# Use the -DHOME line if you want dynamic lookup in /etc/passwd
#HOMENAME=	-DHOME=\"$(HOME)\"
HOMENAME=

NEWSUSR = news
NEWSGRP = news
SPOOLDIR = /usr/spool/news
BATCHDIR = /usr/spool/batch
LIBDIR = /usr/lib/news
BINDIR = /usr/bin
DESTDIR =
UUXFLAGS = -r -z -gd

LNRNEWS = ln
#VMS LNRNEWS = cp	# No links in Eunice
DEBUG =
SCCSID = -DSCCSID
DEFS =	-DRNEWS=\"$(BINDIR)/rnews\" -DSPOOLDIR=\"$(SPOOLDIR)\" \
	-DBATCHDIR=\"$(BATCHDIR)\" -DLIBDIR=\"$(LIBDIR)\" \
	-DBINDIR=\"$(BINDIR)\" -DNEWSUSR=\"$(NEWSUSR)\" \
	-DNEWSGRP=\"$(NEWSGRP)\"

# Order is important here
INSCOMPRESS=
COMPRESS= compress

VFORK=-Dvfork=fork


IBMFLAGS =
CFLAGS = ${DEBUG} -O $(IBMFLAGS) -DUSG -Dindex=strchr -Drindex=strrchr ${DEFS} ${SCCSID} ${VFORK}
LFLAGS = ${DEBUG} -s -i $(IBMFLAGS)
LIBS =
LINTFLAGS = ${DEFS}

TERMLIB = -lcurses
FIXACTIVE =

#VMS TERMLIB = -ltrmlib
#VMS LIBS = -ldbm
#VMS CFLAGS = ${DEBUG} -O -DDBM ${DEFS} -DVMS ${SCCSID} ${VFORK}
#VMS MISC = uname.o
#VMS LINTFLAGS = -chba -DDBM -DVMS ${DEFS}
#VMS VFORK=
#VMS FIXACTIVE = /etc/vmstounix $(LIBDIR)/active

MISC = ftime.o
OBJECTS = funcs.o funcs2.o getdate.o header.o ndir.o $(MISC)
IOBJECTS = inews.o ifuncs.o iextern.o control.o fullname.o \
	ipathinit.o $(OBJECTS)

POBJECTS = postnews.o rextern.o rpathinit.o funcs.o $(MISC)
ROBJECTS = readnews.o rfuncs.o rfuncs2.o rextern.o readr.o \
	process.o rpathinit.o digest.o $(OBJECTS)
VOBJECTS = readnews.o rfuncs.o rfuncs2.o rextern.o process.o rpathinit.o \
	$(OBJECTS) visual.o virtterm.o
EXPOBJS = expire.o header.o funcs.o getdate.o iextern.o epathinit.o \
	funcs2.o ndir.o $(MISC)

SRCS = funcs.c funcs2.c header.c
ISRCS = inews.c ifuncs.c iextern.c control.c fullname.c  $(SRCS)
PSRCS = postnews.c rextern.c funcs.c
RSRCS = readnews.c rfuncs.c rfuncs2.c rextern.c readr.c process.c  \
	digest.c $(SRCS)
VSRCS = readnews.c rfuncs.c rfuncs2.c rextern.c process.c $(SRCS) \
	visual.c virtterm.c
ESRCS = expire.c header.c funcs.c iextern.c \
	funcs2.c
OSRCS = uurec.c recnews.c sendnews.c batch.c unbatch.c caesar.c \
	recmail.c compress.c

UTILS = inews uurec recnews sendnews expire batch unbatch caesar recmail \
	encode decode $(COMPRESS)
SCRIPTS = sendbatch rmgroup checkgroups
OTHERS = $(UTILS) $(SCRIPTS)
COMMANDS = readnews checknews postnews vnews

# dependencies
all: $(OTHERS) $(COMMANDS)

install: all help vnews.help
	-mkdir $(DESTDIR)$(BINDIR)
	-mkdir $(DESTDIR)$(LIBDIR)
	cp $(COMMANDS) $(DESTDIR)$(BINDIR)
	-cd $(DESTDIR)$(BINDIR); \
		strip $(COMMANDS); \
		chown $(NEWSUSR) $(COMMANDS); \
		chgrp $(NEWSGRP) $(COMMANDS); \
		chmod 755 $(COMMANDS)
	cp help vnews.help $(OTHERS) $(DESTDIR)$(LIBDIR)
	cd $(DESTDIR)$(LIBDIR); \
		strip $(UTILS); \
		chown $(NEWSUSR) $(OTHERS); \
		chgrp $(NEWSGRP) $(OTHERS); \
		chmod 755 $(OTHERS)
	-rm -f $(DESTDIR)$(BINDIR)/rnews $(DESTDIR)$(BINDIR)/inews
	${LNRNEWS} $(DESTDIR)$(LIBDIR)/inews $(DESTDIR)$(BINDIR)/rnews
	chown $(NEWSUSR) $(DESTDIR)$(LIBDIR)/inews
	chgrp $(NEWSGRP) $(DESTDIR)$(LIBDIR)/inews
	chmod 6755 $(DESTDIR)$(LIBDIR)/inews $(DESTDIR)$(BINDIR)/rnews
	$(INSCOMPRESS)
#VMS 	vms -v @euninstal

defs.h:	defs.dist localize.sh Makefile.dst Makefile
	sh localize.sh
	@echo Localize has been run. Restart the make.
	@exit 1

Makefile: localize.sh defs.dist Makefile.dst
	sh localize.sh
	@echo Localize has been run. Restart the make.
	@exit 1

update: install.sh makeactive.sh
	sh install.sh $(SPOOLDIR) $(LIBDIR) $(BINDIR) $(NEWSUSR) $(NEWSGRP) $(OSTYPE)
	chmod 6755 $(LIBDIR)/inews

clean:
	rm -f $(COMMANDS) $(OTHERS) *.o a.out
	rm -f core index errs getdate.c

lint:  ilint vlint rlint elint plint olint
ilint: defs.h params.h iparams.h header.h $(ISRCS) pathinit.c
	lint $(LINTFLAGS) $(ISRCS) -DINEW pathinit.c
vlint: defs.h params.h rparams.h header.h $(VSRCS) pathinit.c
	lint $(LINTFLAGS) $(VSRCS) -DREAD pathinit.c
rlint: defs.h params.h rparams.h header.h $(RSRCS) pathinit.c
	lint $(LINTFLAGS) $(RSRCS) -DREAD pathinit.c
elint: defs.h params.h iparams.h header.h $(ESRCS) pathinit.c
	lint $(LINTFLAGS) $(ESRCS) -DEXP pathinit.c
plint: defs.h params.h iparams.h header.h $(PSRCS) pathinit.c
	lint $(LINTFLAGS) $(PSRCS) -DREAD pathinit.c

olint: defs.h params.h iparams.h header.h $(OSRCS)
	for i in $(OSRCS); do lint $(LINTFLAGS) $$i; done

inews:  $(IOBJECTS)
	$(CC) $(LFLAGS) $(IOBJECTS) -o inews $(LIBS)  
#VMS 	vms link/notrace inews.o,ifuncs.o,iextern.o,control.o, \
#VMS 	fullname.o,ipathinit.o,funcs.o,funcs2.o,getdate.o,header.o, \
#VMS 	ndir.o,uname.o,lib:libdbm/lib,lib:libc/lib/exe=inews.

readnews:  $(ROBJECTS)
	$(CC) $(LFLAGS) $(ROBJECTS) -o readnews $(LIBS)
#VMS 	mv readnews.exe readnews

funcs.o:  funcs.c params.h defs.h header.h Makefile
	$(CC) $(CFLAGS) -c funcs.c

funcs2.o:  funcs2.c params.h defs.h header.h Makefile
	$(CC) $(CFLAGS) -c funcs2.c

getdate.o:  getdate.y defs.h
	@echo "expect 8 shift/reduce conflicts"
	yacc getdate.y
	mv y.tab.c getdate.c
	$(CC) $(CFLAGS) -c getdate.c
	-rm -f getdate.c

inews.o:  inews.c iparams.h defs.h params.h header.h Makefile
	$(CC) $(CFLAGS) -c inews.c

ifuncs.o:  ifuncs.c iparams.h defs.h params.h header.h Makefile
	$(CC) $(CFLAGS) $(HOMENAME) -c ifuncs.c

iextern.o:  iextern.c iparams.h defs.h params.h Makefile header.h
	$(CC) $(CFLAGS) $(HOMENAME) -c iextern.c

postnews: $(POBJECTS)
	$(CC) $(CFLAGS) $(LFLAGS) $(POBJECTS) -o postnews
#VMS 	mv postnews.exe postnews

postnews.o: postnews.c defs.h params.h header.h Makefile
	$(CC) $(CFLAGS) -c postnews.c

readnews.o:  readnews.c rparams.h defs.h params.h header.h Makefile
	$(CC) $(CFLAGS) $(HOMENAME) -c readnews.c

rfuncs.o:  rfuncs.c rparams.h defs.h params.h header.h Makefile
	$(CC) $(CFLAGS) -c rfuncs.c

rfuncs2.o:  rfuncs2.c rparams.h defs.h params.h header.h Makefile
	$(CC) $(CFLAGS) -c rfuncs2.c

rextern.o:  rextern.c rparams.h defs.h params.h Makefile header.h
	$(CC) $(CFLAGS) -c rextern.c

readr.o:  readr.c rparams.h defs.h params.h Makefile ndir.h header.h
	$(CC) $(CFLAGS) -c readr.c

checknews.o:  checknews.c defs.h header.h Makefile params.h
	$(CC) $(CFLAGS) -c checknews.c

vnews:	$(VOBJECTS)
	$(CC) $(LFLAGS) $(VOBJECTS) $(TERMLIB) $(LIBS) -o $@
#VMS 	mv vnews.exe vnews

visual.o:  visual.c rparams.h defs.h params.h ndir.h header.h Makefile
	$(CC) $(CFLAGS) -c visual.c

control.o:  control.c defs.h iparams.h params.h header.h Makefile
	$(CC) $(CFLAGS) -c control.c

logdir.o: logdir.c Makefile
	$(CC) $(CFLAGS) -c logdir.c

uname.o:  uname.c defs.h params.h header.h Makefile
	$(CC) $(CFLAGS) -c uname.c

ndir.o: ndir.c ndir.h defs.h
	$(CC) $(CFLAGS) -c ndir.c

uurec:  uurec.c defs.h Makefile
	$(CC) $(CFLAGS) $(LFLAGS) uurec.c -o uurec
#VMS 	mv uurec.exe uurec

recnews:  recnews.c defs.h header.h Makefile
	$(CC) $(CFLAGS) $(LFLAGS) recnews.c -o recnews
#VMS 	mv recnews.exe recnews

sendnews:  sendnews.o defs.h
	$(CC) $(LFLAGS) sendnews.o -o sendnews
#VMS 	mv sendnews.exe sendnews

batch:  batch.c Makefile defs.h
	$(CC) $(CFLAGS) $(LFLAGS) batch.c -o batch
#VMS 	mv batch.exe batch

unbatch:  unbatch.c Makefile
	$(CC) $(CFLAGS) $(LFLAGS) unbatch.c -o unbatch
#VMS 	mv unbatch.exe unbatch

encode:	encode.c
	$(CC) $(CFLAGS) $(LFLAGS) encode.c -o encode
#VMS 	mv encode.exe encode

decode:	decode.c
	$(CC) $(CFLAGS) $(LFLAGS) decode.c -o decode
#VMS 	mv decode.exe decode

caesar:  caesar.c Makefile
	$(CC) $(CFLAGS) $(LFLAGS) caesar.c -o caesar -lm
#VMS 	mv caesar.exe caesar

compress: compress.c
	$(CC) $(CFLAGS) $(LFLAGS) -o compress compress.c
#VMS 	mv compress.exe compress

recmail:  recmail.c defs.h params.h header.h
	$(CC) $(CFLAGS) $(LFLAGS) recmail.c -o recmail
#VMS 	mv recmail.exe recmail

process.o:  process.c rparams.h defs.h params.h header.h Makefile
	$(CC) $(CFLAGS) -c process.c

checknews:  checknews.o process.o cpathinit.o rextern.o
	$(CC) $(LFLAGS) checknews.o process.o cpathinit.o rextern.o -o checknews
#VMS 	mv checknews.exe checknews

sendbatch: sendbatch.sh Makefile
	sed -e "s%LIBDIR%$(LIBDIR)%g" \
	 -e "s%UUXFLAGS%$(UUXFLAGS)%g" \
	 -e "s%BATCHDIR%$(BATCHDIR)%g" sendbatch.sh > sendbatch
#VMS 	/etc/unixtovms sendbatch

rmgroup: rmgroup.sh Makefile
	sed -e "s%LIBDIR%$(LIBDIR)%g" \
	 -e "s%FIXACTIVE%$(FIXACTIVE)%g" \
	 -e "s%SPOOLDIR%$(SPOOLDIR)%g" rmgroup.sh > rmgroup
#VMS 	/etc/unixtovms rmgroup

checkgroups: checkgroups.sh Makefile
	sed -e "s%LIBDIR%$(LIBDIR)%g" checkgroups.sh > checkgroups
#VMS 	/etc/unixtovms checkgroups

sendnews.o: sendnews.c defs.h
	$(CC) $(CFLAGS) -c sendnews.c

fullname.o: defs.h params.h header.h Makefile
	$(CC) $(CFLAGS) -c fullname.c

expire: $(EXPOBJS)
	$(CC) $(LFLAGS) -o expire $(EXPOBJS) $(LIBS)
#VMS 	mv expire.exe expire

header.o:  header.c header.h defs.h patchlevel.h params.h Makefile
	$(CC) $(CFLAGS) -c header.c

expire.o:  expire.c defs.h Makefile params.h ndir.h header.h Makefile
	$(CC) $(CFLAGS) -c expire.c

digest.o:  digest.c Makefile
	$(CC) $(CFLAGS) -c digest.c

# Some silliness here to get pathinit for both readnews & inews
rpathinit.o:  pathinit.c rparams.h header.h params.h defs.h Makefile
	$(CC) $(CFLAGS) $(HOMENAME) -DREAD -c pathinit.c
	mv pathinit.o rpathinit.o

ipathinit.o:  pathinit.c iparams.h header.h params.h defs.h Makefile
	$(CC) $(CFLAGS) $(HOMENAME) -DINEW -c pathinit.c
	mv pathinit.o ipathinit.o

cpathinit.o:  pathinit.c iparams.h header.h params.h defs.h Makefile
	$(CC) $(CFLAGS) $(HOMENAME) -DCHKN -c pathinit.c
	mv pathinit.o cpathinit.o

epathinit.o:  pathinit.c iparams.h header.h params.h defs.h Makefile
	$(CC) $(CFLAGS) $(HOMENAME) -DEXP -c pathinit.c
	mv pathinit.o epathinit.o

tags:	/tmp
	ctags -w *.h *.c

$(LIBDIR):
	mkdir $(LIBDIR)
	chmod 755 $(LIBDIR)
	chown $(NEWSUSR) $(LIBDIR)
	chgrp $(NEWSGRP) $(LIBDIR)

$(BINDIR):
	mkdir $(BINDIR)
	chmod 755 $(BINDIR)
	chown $(NEWSUSR) $(BINDIR)
	chgrp $(NEWSGRP) $(BINDIR)
