#	@(#)makefile	1.2

xmail:	xmail.c
	rm -f $@
	cc -g -O xmail.c -o $@
	chown bin $@
	chgrp mail $@
	chmod 2755 $@

#The following rule requires you to be superuser
install:	xmail xmail.1
	isroot
	cp xmail /usr/lbin/xmail
	chown bin /usr/lbin/xmail
	chgrp mail /usr/lbin/xmail
	chmod 2755 /usr/lbin/xmail
	cp xmail.1 /usr/man/local/man1/xmail.1
	chown bin /usr/man/local/man1/xmail.1
	chgrp bin /usr/man/local/man1/xmail.1
	chmod 644 /usr/man/local/man1/xmail.1
	touch install

news:	xmail.news
	sed 1d xmail.news >/usr/news/xmail
	chmod 644 /usr/news/xmail
