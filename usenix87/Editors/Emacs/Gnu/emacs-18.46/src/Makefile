CPP = cc -E
#Note: an alternative is  CPP = /lib/cpp

all: xmakefile doall

doall:
	make -f xmakefile ${MFLAGS} all
.RECURSIVE: doall

#This is used in making a distribution.
#Do not use it on development directories!
clean:
	-rm -f paths.h config.h emacs-* temacs xemacs xmakefile core *~ \#* *.o

xemacs: xmakefile doxemacs

doxemacs:
	make -f xmakefile ${MFLAGS} xemacs
.RECURSIVE: doxemacs

temacs: xmakefile dotemacs

dotemacs:
	make -f xmakefile ${MFLAGS} temacs
.RECURSIVE: dotemacs

# If you have a problem with cc -E here, changing
# the definition of CPP above may fix it.
xmakefile: ymakefile config.h
	rm -f xmakefile
	cp ymakefile junk.c
	$(CPP) junk.c | sed -e 's/^#.*//' -e 's/^[ \f	]$$//' | \
	sed -n -e '/^..*$$/p' > xmakefile
	rm -f junk.c
