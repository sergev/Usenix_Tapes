# These programs were distributed by the Computer Systems Research Group,
# University of Toronto, at the Austin USENIX conference in June 1981.

# As far as we can tell, these programs are in the public domain, and
# can be used by anyone, with or without a UNIX license from Bell.

CCFLAGS= -s -O

ALL:	apb banner c calls cptree cu danl home huf lastlog mcv modcheck \
	news p phone po shutdown where wmi x yfr

apb c calls cptree cu danl home huf lastlog modcheck p phone \
po wmi x yfr :
	cc $(CCFLAGS) $@.c
	mv a.out $@

banner:
	cd banner; make

news:	news.c
	cc $(CCFLAGS) news.c
	mv a.out news
	echo "To make the news command work, you must add a 'news' userid to /etc/passwd"

shutdown:	shutdown.c
	cc $(CCFLAGS) shutdown.c
	mv a.out shutdown
	chown root shutdown
	chmod u+s shutdown

mcv	: mcv mcv.script
	cp mcv.script /usr/lib

where	: where tocrc
	cp tocrc /usr/man/man0
	echo "0 9 * * 0 /usr/man/man0/tocrc all > /dev/null" >> /usr/lib/crontab
