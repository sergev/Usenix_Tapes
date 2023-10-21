# This makefile will assemble and link macro.v6 and linkr.v6.
#
# macro and linkr are shell files in /usr/local which invoke
# the pdp11 compatibility package to run version 6 binaries
# such as macro.v6 and linkr.v6
#
BINDIR=		/usr/local

MACOBJS=	exec.obj macro.obj code.obj expr.obj fltg.obj getl.obj\
		lout.obj mac.obj srch.obj xlat.obj misc.obj pst.obj xpcor.obj

LNKOBJS=	exec.obj linkr.obj srch.obj misc.obj xpcor.obj

CRFOBJS=	cref.obj srch.obj misc.obj

MACOBJS:	/etc/macro/sysmac.sml at.sml

LNKOBJS:	/etc/macro/sysmac.sml at.sml

CRFOBJS:	/etc/macro/sysmac.sml at.sml

all:		macro.v6 linkr.v6 macxrf.v6 macxrf

macro.v6:	${MACOBJS}	 

	linkr -ls -xs:4  \
		exec macro code expr fltg getl \
		lout mac srch xlat misc pst xpcor -na:macro
	mv macro.out macro.v6


linkr.v6:	${LNKOBJS}

	linkr -ls -xs:4 \
		exec linkr srch misc xpcor -na:linkr
	mv linkr.out linkr.v6

macxrf.v6:	${CRFOBJS}

	linkr -ls -xs:4  \
		cref srch misc -na:macxrf
	mv macxrf.out macxrf.v6

macxrf:		macxrf.c

	cc -O -o macxrf macxrf.c



code.obj:	code.m11
	macro -ls -xs:4  code.m11
cref.obj:	cref.m11
	macro -ls -xs:4  cref.m11
exec.obj:	exec.m11
	macro -ls -xs:4  exec.m11
expr.obj:	expr.m11
	macro -ls -xs:4  expr.m11
fltg.obj:	fltg.m11
	macro -ls -xs:4  fltg.m11
getl.obj:	getl.m11
	macro -ls -xs:4  getl.m11
linkr.obj:	linkr.m11
	macro -ls -xs:4  linkr.m11
lout.obj:	lout.m11
	macro -ls -xs:4  lout.m11
mac.obj:	mac.m11
	macro -ls -xs:4  mac.m11
macro.obj:	macro.m11
	macro -ls -xs:4  macro.m11
misc.obj:	misc.m11
	macro -ls -xs:4  misc.m11
pst.obj:	pst.m11
	macro -ls -xs:4  pst.m11
srch.obj:	srch.m11
	macro -ls -xs:4  srch.m11
xlat.obj:	xlat.m11
	macro -ls -xs:4  xlat.m11
xpcor.obj:	xpcor.m11
	macro -ls -xs:4  xpcor.m11

compare:

	cmp macro.v6 ${BINDIR}/macro.v6
	cmp linkr.v6 ${BINDIR}/linkr.v6
	cmp macxrf.v6 ${BINDIR}/macxrf.v6

install:

	mv macro.v6 ${BINDIR}/macro.v6
	mv linkr.v6 ${BINDIR}/linkr.v6
	mv macxrf.v6 ${BINDIR}/macxrf.v6
	mv macxrf ${BINDIR}/macxrf

clean:
	rm *.lst *.map *.obj


setup:
	mkdir /etc/macro
	cp sysmac.sml /etc/macro
