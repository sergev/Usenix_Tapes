#
# @(#)makefile 1.1 85/05/30 MITRE
#

DESTDIR=/usr/local/lib
OBJSLIB=cim_change.o cim_size.o csr_change.o csr_init.o ttysw_main.o ttyvt100.o vt100fonts.o vt100keys.o




all: nlibsuntool.a

nlibsuntool.a:  $(OBJSLIB)
	touch *.o
	cp /usr/lib/libsuntool.a nlibsuntool.a
	ar d nlibsuntool.a ttyansi.o
	ar rv nlibsuntool.a $?
	ranlib nlibsuntool.a


