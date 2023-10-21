#
# Makefile for all of InterViews
#
# The basic strategy is to first build the core library,
# then the other libraries, then the binaries.
#
# The variables m and w specify the target machine
# (e.g., SUN3, VAX) and window system (e.g., X10, X11).
#

m = SUN3
w = X11
d = ${m}.${w}

all: install-lib install-libgraphic install-libtext install-bin

install-lib:
	cd lib/${d}; make install

install-libgraphic:
	cd libgraphic/${m}; make install

install-libtext:
	cd libtext/${m}; make install

install-bin: alert dclock graphics idraw istat iwm mailbox pages squares

alert:
	cd bin/alert/${d}; make install

dclock:
	cd bin/dclock/${d}; make install

graphics:
	cd bin/graphics/${d}; make install

idraw:
	-cd bin/idraw/${d}; make ${w}-install

istat:
	cd bin/istat/${d}; make install

iwm:
	cd bin/iwm/${d}; make install

mailbox:
	cd bin/mailbox/${d}; make install

pages:
	cd bin/pages/${d}; make install

squares:
	cd bin/squares/${d}; make install

clean: clean-lib clean-libgraphic clean-libtext clean-bin

clean-lib:
	cd lib/${d}; make clean

clean-libgraphic:
	cd libgraphic/${m}; make clean

clean-libtext:
	cd libtext/${m}; make clean

clean-bin:
	cd bin/alert/${d}; make clean
	cd bin/dclock/${d}; make clean
	cd bin/graphics/${d}; make clean
	-cd bin/idraw/${d}; make clean
	cd bin/istat/${d}; make clean
	cd bin/iwm/${d}; make clean
	cd bin/mailbox/${d}; make clean
	cd bin/pages/${d}; make clean
	cd bin/squares/${d}; make clean

depend: depend-lib depend-libgraphic depend-libtext depend-bin

depend-lib:
	cd lib; genmake ${m} ${w}

depend-libgraphic:
	cd libgraphic; genmake ${m}

depend-libtext:
	cd libtext; genmake ${m}

depend-bin:
	cd bin/alert; genmake ${m} ${w}
	cd bin/dclock; genmake ${m} ${w}
	cd bin/graphics; genmake ${m} ${w}
	cd bin/istat; genmake ${m} ${w}
	cd bin/iwm; genmake ${m} ${w}
	cd bin/mailbox; genmake ${m} ${w}
	cd bin/pages; genmake ${m} ${w}
	cd bin/squares; genmake ${m} ${w}

scratch: clean depend all
