case $CONFIG in
    '') . config.sh ;;
esac
echo "Extracting Makefile (with variable substitutions)"
cat >Makefile <<!GROK!THIS!
####################################################################
# makefile for uumail & uupath
# program to integrate with pathalias created uucpmap databases
# programs originally developed by Jeff Donnelly
# updated to use pathalias database by Stan Barber
# $Header: Makefile.SH,v 1.2 86/12/15 13:27:53 sob Exp $
#***************************************************************************
# This work in its current form is Copyright 1986 Stan Barber
# with the exception of resolve, gethostname and the original getpath which
# as far as I know are in the Public Domain. This software may be distributed
# freely as long as no profit is made from such distribution and this notice
# is reproducted in whole.
# ***************************************************************************
# 
###############################################################
.SUFFIXES: .c,v .h,v

CC= $cc

CP= $cp

RM= $rm

LIBS= $libdbm $ndirlib

BINDIR=$bindir

UUCPDIR=$lib

MANDIR=$mansrc

UUMAIL=$(UUCPDIR)/uumail

REALUUX=/usr/bin/uux

CFLAGS=-O -DUUMAIL='"$(UUMAIL)"' -DREALUUX='"$(REALUUX)"'
!GROK!THIS!
cat >> Makefile <<'!NO!SUBS!'
SHELL=/bin/sh

.c,v.c:
	co -q $*.c

.h,v.h:
	co -q $*.h

all: uumail rmail address uux

cobj=getpath.o resolve.o

uobj= uumail.o gethostnam.o alias.o deadletter.o $(cobj)

robj= rmail.o gethostnam.o

aobj= address.o $(cobj)

obj= $(aobj) $(uobj) $(robj)

uumail: $(uobj)
	$(CC) $(CFLAGS) $(uobj) -o uumail $(LIBS)

address:$(aobj)
	$(CC) $(CFLAGS) $(aobj) -o address $(LIBS)

rmail: $(robj)
	$(CC) $(CFLAGS) $(robj) -o rmail $(LIBS)

install: uumail address palias
	$(CP) address $(BINDIR)
	$(CP) uumail $(UUCPDIR)
	$(CP) uumail.8 $(MANDIR)/man8
	$(CP) address.1 $(MANDIR)/man1
	ln $(MANDIR)/man8/uumail.8 $(MANDIR)/man1/uupath.1
	if [ ! -r $(UUCPDIR)/palias ]; then cp palias $(UUCPDIR); fi
	@echo "To install rmail in place of the current rmail, type"
	@echo "make mailer"
	@echo "To intercept uux commands, you need to be sure"
	@echo "that you correctly specified REALUUX in makefile"
	@echo "Then type make fakeuux"
	ln $(UUCPDIR)/uumail $(BINDIR)/uupath

mailer: rmail
	make install
	$(RM) -rf /bin/rmail
	$(CP) rmail /bin/rmail
	
fakeuux: uux
	make install
	$(CP) /usr/bin/uux $(REALUUX)
	$(CP) uux /usr/bin/uux
	chmod 6755 /usr/bin/uux

lint:
	lint $(CFLAGS) getpath.c uumail.c gethostnam.c resolve.c alias.c

clean: 
	$(RM) -f *.o uumail address rmail uux

doc: uumail.8 address.1
	nroff -man uumail.8 >uumail.cat; nroff -man address.1 >address.cat


# AUTOMATICALLY GENERATED MAKE DEPENDENCIES--PUT NOTHING BELOW THIS LINE
$(obj):
	@ echo "You haven't done a "'"make depend" yet!'; exit 1
!NO!SUBS!
$eunicefix Makefile
