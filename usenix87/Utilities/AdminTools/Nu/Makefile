# This Makefile courtesy of nsc!glenn (Glenn Skinner)

SHELLFILES	= nu1.sh nu2.sh nu3.sh nu4.sh

nu:	nu.c
	cc -O -o nu nu.c -ldbm

install: nu
	-mkdir /etc/nulib
	install -s nu /etc
	for i in ${SHELLFILES}; do install -c -m 555 $$i /etc/nulib; done
	install -c -m 444 nu.cf.debug /etc/nulib
	install -c -m 444 nu.cf.real  /etc/nulib
	-ln -s /etc/nulib/nu.cf.real /etc/nu.cf
	install -c -m 444 nu.8 /usr/man/man8
	@echo Make sure you edit /etc/nulib/nu.cf.\* for your site
