# make all	to compile and build Emacs
# make install	to install it
# make tags	to update tags tables
#
# make clean	to delete everything that wasn't in the distribution
#	This is a very dangerous thing to do!

# Where to install things
LIBDIR= /usr/local/emacs
BINDIR= /usr/local/bin
MANDIR= /usr/man/man1

# Flags passed down to subdirectory makefiles.
MFLAGS=

# Subdirectories to make recursively.  `lisp' is not included
# because the compiled lisp files are part of the distribution
# and you cannot remake them without installing Emacs first.
SUBDIR= etc src

# Subdirectories to install
COPYDIR= etc info lisp

# Subdirectories to clean
CLEANDIR= ${COPYDIR} lisp/term

all:	src/paths.h ${SUBDIR} lock

src/paths.h: Makefile
	/bin/sed 's;/usr/local/emacs;${LIBDIR};' < src/paths.h-dist > src/paths.h

src:	etc

.RECURSIVE: ${SUBDIR}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS} all

install: all mkdir
	tar cf - ${COPYDIR} | (cd ${LIBDIR}; tar xpBvf - )
	for i in ${CLEANDIR}; do \
		(rm -rf ${LIBDIR}/$$i/RCS; \
		 rm -f ${LIBDIR}/$$i/\#*; \
		 rm -f ${LIBDIR}/$$i/*~); \
	done
	install -c -s -g kmem -m 2755 etc/loadst ${LIBDIR}/etc/loadst
	install -c -s etc/etags ${BINDIR}/etags
	install -c -s etc/ctags ${BINDIR}/ctags
	install -c -s -m 1755 src/xemacs ${BINDIR}/xemacs
	install -c -m 444 etc/emacs.1 ${MANDIR}/emacs.1
	-rm -f ${BINDIR}/emacs
	mv ${BINDIR}/xemacs ${BINDIR}/emacs

mkdir: FRC
	-mkdir ${LIBDIR}
	-chmod 777 ${LIBDIR}

clean:
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

lock:
	-mkdir ${LIBDIR}/lock
	chmod 777 ${LIBDIR}/lock

FRC:

tags:	etc
	cd src; ../etc/etags *.{c,h} ../lisp/*.el ../lisp/term/*.el
