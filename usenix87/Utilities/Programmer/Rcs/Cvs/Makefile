#	This file is part of the Concurrent Versions System CVS.
#	Written by Dick Grune, Vrije Universiteit, Amsterdam.
#	$Header: Makefile,v 1.24 86/06/22 18:12:37 dick Exp $

CVSBIN = /user1/dick/bin#		# where to install the CVS programs
CVSLIB = /user1/dick/lib/cvs#		# where to install the CVS auxiliaries
CVSMAN = /user1/dick/man#		# where to install the CVS manual
RCSBIN = /usr/new#			# where the RCS binaries reside

INF =	READ_ME Makefile Install cvs.1
PRG =	AE CM CV DF GC LS RM RV SV UV
AUX =	BE.aux CA.aux CI.aux CS.aux EF.aux FN.aux LR.aux ND.aux NR.aux \
	OP.aux RG.aux SC.aux SL.aux VN.aux VT.aux

what:
	@echo "Call is: make [ install | print | shar | clean ]"

install:	install.files $(CVSMAN)/cvs.1

install.files:
	for F in $(PRG); do ./Install $$F $(CVSBIN) $(CVSLIB) $(RCSBIN); done
	for F in $(AUX); do ./Install $$F $(CVSLIB) $(CVSLIB) $(RCSBIN); done

$(CVSMAN)/cvs.1:	cvs.1
	cp cvs.1 $(CVSMAN)/cvs.1

# create a (composite) shar file shar[12]
shar:	shar1 shar2

shar1:	$(INF) $(PRG) Makefile
	shar $(INF) $(PRG) >shar1

shar2:	$(AUX) Makefile
	shar $(AUX) >shar2

# the whole set:
FLS =	$(INF) ChangeLog $(PRG) $(AUX)

print:	$(FLS) Makefile
	pr $(FLS) >print

clean:
	rm -f shar[12] print

