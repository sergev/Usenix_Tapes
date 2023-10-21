# Makefile for the Digest system...
#
# This entire Digest system is;
#   (C) Copyright 1986, Dave Taylor, Hewlett-Packard
#

# To use, merely type "make all"

SHELL=/bin/csh

CFILES= readword.c center.c mydate.c
SCRIPTS= digest.all digest.awk digest.tail issue mail.digest mail_issue \
	volume_number

CC=/bin/cc
CFLAGS=-O

all:	readword .DIGESTNAME .DIGESTADDR .MODERATOR .MOD_ADDR \
	center mydate ${SCRIPTS}
	@chmod a+rx digest.all issue mail.digest mail_issue
	@echo digest system ready to go...

readword : readword.c
	@echo Creating readword program for interactive section of Makefile
	@echo " "
	${CC} ${CFLAGS} readword.c -o readword

center : center.c
	${CC} ${CFLAGS} center.c -o center
	
mydate: mydate.c
	${CC} ${CFLAGS} mydate.c -o mydate

.DIGESTNAME: 
	@echo " "
	@echo -n "Please enter the name of the digest: "
	@readword .DIGESTNAME

.DIGESTADDR:
	@echo -n "Please enter the alias you'll use for sending the digest: "
	@readword .DIGESTADDR

.MODERATOR: 
	@echo -n "Please enter the full name of the moderator: "
	@readword .MODERATOR

.MOD_ADDR:
	@echo -n "Please enter the mailing address for mail to the moderator: "
	@readword .MOD_ADDR
	@echo " "

digest.tail: 
	@echo 'Creating digest.tail...'
	@echo " " > digest.tail
	@echo "-------------------------------------" >> digest.tail
	@echo " " >> digest.tail
	@echo \
        "To join this group or have your thoughts in the next issue, please" \
	>> digest.tail
	@echo \
	"send electronic mail to `cat .MODERATOR` at the following address;" \
	>> digest.tail
	@echo " " >> digest.tail
	@echo "		`cat .MOD_ADDR`" >> digest.tail
	@echo " " >> digest.tail
	@echo "The views expressed in `cat .DIGESTNAME`" >> digest.tail
	@echo "are those of the individual authors only.">> digest.tail
	@echo " " >> digest.tail
	@echo "*********************" >> digest.tail
	@echo "End of `cat .DIGESTNAME`" >> digest.tail
	@echo "*********************" >> digest.tail

issue: issue.master 
	@echo "Creating issue script..."
	@echo " " > issue
	@echo " digest_name='`cat .DIGESTNAME`'" >> issue
	@cat issue.master >> issue

mail.digest: mail.digest.m 
	@echo "Creating mail.digest..."
	@echo " " > mail.digest
	@echo " digest_name='`cat .DIGESTNAME`'" >> mail.digest
	@cat mail.digest.m >> mail.digest

mail_issue: mail.issue.m 
	@echo "Creating mail_issue..."
	@echo " " > mail_issue
	@echo " digest_name='`cat .DIGESTNAME`'" >> mail_issue
	@echo "          to='`cat .DIGESTADDR`'" >> mail_issue
	@cat mail.issue.m >> mail_issue

volume_number: 
	@echo 'Creating volume_number file...'
	@echo 1 > volume_number

clean:
	/bin/rm -f .DIGESTNAME .DIGESTADDR .MODERATOR \
	.MOD_ADDR digest.tail issue digest.title \
	mail.digest mail_issue volume_number center mydate readword

