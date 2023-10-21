##
##  SOURCE-SHIPPING TOOLS MAKEFILE
##  $Header: Makefile,v 1.16 87/03/18 13:12:39 rs Exp $
##

##  Edit appropriately.
CFLAGS	= -O

##  Use these two lines if you use ranlib...
RANLIB	= ranlib lib.a
AR_OBJ	= $(LOBJ)
##  ...or use these two if you don't.
#AR_OBJ	= `lorder $(LOBJ) | tsort`
#RANLIB	= @echo

##  Use this line if you have ctags (free from mod.sources)...
CTAGS	= ctags -tw $(SRCS) $H
# ...or this line if you don't.
#CTAGS	= @echo

##  Where executables should be put.
DESTDIR	= /usr/local/bin

##  The manpage for "foo" goes in $(MANDIR)/foo.$1
MANDIR	= /usr/man/man1
1	= 1
#MANDIR	= /usr/man/u_man/manl
#1	= 1L

##
##  END OF CONFIGURATION SECTION
##

##  Shorthands for sources, objects, etc.
H	= shar.h
L	= lib.a
PROGS	= findsrc makekit shar unshar shell
SRCS	= findsrc.c getopt.c makekit.c mem.c shar.c unshar.c
LSRC	= cwd.c getopt.c host.c sysfuncs.c glue.c parser.c mem.c
LOBJ	= cwd.o getopt.o host.o sysfuncs.o glue.o parser.o mem.o
MAN	= findsrc.$1 makekit.$1 shar.$1 unshar.$1 shell.$1


all:		programs tags

install:	newprog newman


##  You might want to change these actions...
newprog:	$(PROGS)
	cd $(DESTDIR) ; rm -f $(PROGS)
	cp $(PROGS) $(DESTDIR)
	cd $(DESTDIR) ; strip $(PROGS) ; chmod 755 $(PROGS)
#	cd $(DESTDIR) ; /etc/chown root $(PROGS)

newman:		$(MAN)
	cd $(MANDIR) ; rm -f $(MAN)
	cp $(MAN) $(MANDIR)


##  CREATING MANPAGES
.SUFFIXES:	.$1 .man
.man.$1:
	cp $< $@
	chmod 444 $@

##  PROGRAM TARGETS
programs:	$(PROGS)

findsrc:	findsrc.o $H $L
	$(CC) $(CFLAGS) -o findsrc findsrc.o $L

makekit:	makekit.o $H $L
	$(CC) $(CFLAGS) -o makekit makekit.o $L

shar:		shar.o $H $L
	$(CC) $(CFLAGS) -o shar shar.o $L

shell:		shell.o $H $L
	$(CC) $(CFLAGS) -o shell shell.o $L

##  Special -- gotta build these from the .c files...
unshar:		unshar.c $H $L
	$(CC) $(CFLAGS) -o unshar unshar.c $L
unshar.safe:	unshar.c $H $L
	$(CC) $(CFLAGS) -DUSE_MY_SHELL -o unshar unshar.c $L


##  UTILITY TARGETS
lib.a:		$(LOBJ)
	@rm -f lib.a
	ar r lib.a $(AR_OBJ)
	$(RANLIB)
$(LOBJ):	$H

tags:		$(SRCS) $H
	$(CTAGS)

clean:
	rm -f *.[oa] *.$1 *.BAK $(PROGS) unshar.safe
	rm -f lint lib.ln tags core foo a.out


##  LINT (probably only good for 4.[23]BSD Unices)
#LINTF	= -p -ahb
LINTF	= -ahb
lint:		programs lib.ln
	exec lint $(LINTF)u  >lint $(LSRC)
	exec lint $(LINTF)  >>lint findsrc.c lib.ln
	exec lint $(LINTF)  >>lint makekit.c lib.ln
	exec lint $(LINTF)  >>lint shar.c    lib.ln
	exec lint $(LINTF)  >>lint shell.c   lib.ln
	exec lint $(LINTF)  >>lint unshar.c  lib.ln
#	exec lint $(LINTF) -DUSE_MY_SHELL >>lint unshar.c  lib.ln
	

lib.ln:		$(LSRC)
	lint -CX $(LSRC)
	mv llib-lX.ln lib.ln
