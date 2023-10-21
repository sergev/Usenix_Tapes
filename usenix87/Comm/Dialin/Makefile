#
# Makefile for dialin program to make /usr/spool/uucp/LCK.. files
# to keep dial in lines from getting dropped by outgoin uucp or cu.
#

# you probably need to be root to do the install.

dialin:	dialin.o
	ld /lib/crt0s.o /lib/shlib.ifile dialin.o -o dialin
	strip dialin
	@size dialin
	@ls -l dialin

install: dialin
	cp dialin /usr/bin
	chmod 755 /usr/bin/dialin
	chgrp bin /usr/bin/dialin
	chown bin /usr/bin/dialin
	@echo 'Now edit the file /etc/profile -- see the source'

