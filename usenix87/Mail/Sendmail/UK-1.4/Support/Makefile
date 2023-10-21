#
#  Makefile for assorted programs related to Sendmail.
#
#	@(#)Makefile	UK-1.1 sendmail configuartion		22/4/85
#
CFLAGS=-O
#CFLAGS=-O -Dvoid=int   # some pdp11's need this
X25FLAGS=-I/usr/src/local/x25/usr/src/

uucp:	rmail
	cp rmail /bin/rmail

bitnet:	damail
	cp rmail /usr/lib/rscs/damail

rmail:	rmail.c
	cc $(CFLAGS) -o rmail

damail: damail.c
	cc $(CFLAGS) -o damail


janet:	mailer hhsend
	cp mailer /usr/lib/x25
	chown root /usr/lib/x25/mailer
	chmod 755 /usr/lib/x25/mailer
	cp hhsend /usr/lib/x25
	chown root /usr/lib/x25/hhsend
	chmod 4755 /usr/lib/x25/hhsend

mailer: mailer.c
	cc $(CFLAGS) mailer.c -lx25

hhsend: hhsend.c
	cc $(CFLAGS) $(X25FLAGS) hhsend.c
