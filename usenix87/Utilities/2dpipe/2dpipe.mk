INSDIR=/usr/local
MANDIR=/usr/man/l_man/man1
CATDIR=/usr/catman/l_man/man1

install: cmd man
cmd: 
	cp 2dpipe.sh $(INSDIR)/0
	-ln $(INSDIR)/0 $(INSDIR)/1
	-ln $(INSDIR)/0 $(INSDIR)/2
	chmod 755 $(INSDIR)/[012]
	chown bin $(INSDIR)/[012]
	chgrp bin $(INSDIR)/[012]

man:
	cp 2dpipe.1 $(MANDIR)
	chmod 644 $(MANDIR)/2dpipe.1
	chown bin $(MANDIR)/2dpipe.1
	chgrp bin $(MANDIR)/2dpipe.1
	nroff -man -Tlp 2dpipe.1 > /tmp/2dpipe.1
	-rm /tmp/2dpipe.1.z
	pack /tmp/2dpipe.1
	mv /tmp/2dpipe.1.z $(CATDIR)
	chmod 644 $(CATDIR)/2dpipe.1.z
	chown bin $(CATDIR)/2dpipe.1.z
	chgrp bin $(CATDIR)/2dpipe.1.z
