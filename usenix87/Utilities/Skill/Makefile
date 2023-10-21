#
#	M A K E F I L E			1.1		Jeff Forys
#
#	1) Add one (and only one) of these to CFLAGS below depending on
#	   the system you are using:
#		BSD4_2	- 4.2 BSD system
#		BSD4_3	- 4.3 BSD system
#		SUN2	- Sun 2 workstation
#		SUN3	- Sun 3 workstation
#
#	2) KMEM should be set the the group permission of "/dev/kmem".
#
#	3) set DESTDIR to what directory you want the executable put in.
#
#	4) set MAN1 to where you want the manuap page placed.
#
#	5) type 'make install'
#

CFLAGS= -O -DBSD4_2
KMEM=kmem
DESTDIR=/usr/local/bin
MAN1=/usr/man/manl

skill:	skill.o
	cc ${CFLAGS} -o skill skill.o

install: skill
	install -c -s skill ${DESTDIR}
	chgrp kmem ${DESTDIR}/skill
	chmod g+s ${DESTDIR}/skill

man:
	cp skill.1 ${MAN1}

clean:
	rm -f *.o skill
