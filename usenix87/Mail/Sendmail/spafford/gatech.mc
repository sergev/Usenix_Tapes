############################################################
############################################################
#####
#####		SENDMAIL CONFIGURATION FILE FOR GATECH
#####
#####	This one is the big daddy.  There is no "upstairs"
#####	to bounce a message to -- except perhaps the CSnet
#####
#####	$Header: gatech.mc,v 5.3 85/10/27 16:07:45 spaf Release $
#####
############################################################
############################################################



############################################################
###	local info
############################################################

# internet hostname
Cwgatech ga-tech georgia-tech gt-tech gt-gatech

# override SMTP hostname to match Arpanet name
Dj$w.CSNET

# Our UUCP hostname(s)
DUgatech
CUgatech gt-tech gt-gatech georgia-tech

include(csether.m4)

# Defined Gateway sites and so on.  Hosts are listed in files.
#
# 	ARPA Gateway
DAcsnet-relay
FA/usr/lib/mail/arpa.hosts %s
#
#	BITNET gateway
DBwiscvm.arpa
FB/usr/lib/mail/bitnet.hosts %s
#
#	 CSNET gateway
DCcsnet-relay
FC/usr/lib/mail/csnet.hosts %s
#
#       Gateway to Dec E-Net
DEdecwrl.dec.com
FE/usr/lib/mail/decnet.hosts %s
#
#	Gateway to Mailnet.
DMmit-multics.arpa
FM/usr/lib/mail/mailnet.hosts %s
#
#	UUCP network
#  (no gateway host)
FX/usr/lib/mail/uucp.hosts %s
#
#	OZ gateway
#	(no list of sites)
DZmunnari.uucp


# we have full sendmail support here
Oa

include(gtbase.m4)

################################################
###  Machine dependent part of ruleset zero  ###
################################################


# 	Resolve names that can go via the ethernet
R$*<@$*$=S.LOCAL>$*		$#ether$@$3$:$1<@$2$3.$D>$4	user@etherhost

# 	Resolve local UUCP links (all others)
R$+<@$+.$=T.UUCP>	$1<@$2.$3>			a.arpa.uucp -> a.arpa
R<@$+.$-.UUCP>:$+	$#uucp$@$2$:@$1.$2.UUCP:$3	@host.domain.UUCP: ...
R<@$-.UUCP>:$+		$#uucp$@$1$:$2			@host.UUCP: ...
R$+<@$+.$-.UUCP>	$#uucp$@$3$:$1@$2.$3.UUCP	user@host.domain.UUCP
R$+<@$-.UUCP>		$#uucp$@$2$:$1			user@host.UUCP

#
#	Resolution of the CSNET, ARPA, BITNET and MAILNET domains should really
#	have some sort of provision for addresses of the form:
#	"@domain.XXX:rest-of-address" similar to the UUCP stuff
#

#	Resolve ARPA names - these go by way of the PMDF mailer.
#       If we had an Arpa link, we'd use the TCP mailer instead.
R$+<@$*.$=K>		$#pmdf$@$A$:$1<@$2.$3>		user@site.ARPA

# Current: send BITNET mail to a known gatewaying host (wiscvm.arpa)
R$+<@$*.BITNET>		$@$>0$1%$2.BITNET<@$B>		user@site.BITNET

#	Resolve mail to the CSNET domain
#		make sure to leave the "csnet" in the address
R$+<@$*.CSNET>		$#pmdf$@$C$:$1<@$2.CSNET>	user@site.CSNET

#	Resolve addresses to the MAILNET domain - these are handled
#		by the site in the $M macro.  We merely re-iterate rule 0
#		to get to the site specified by $M.
R$+<@$*.MAILNET>	$@$>0$1%$2.MAILNET<@$M>		user@site.MAILNET

#	Resolve DEC E-Net addresses
R$+<@$*.DEC>		$@$>0$1%$2.DEC<@$E>		user@site.DEC

#	Resolve OZ addresses
R$+<@$*.OZ>		$@$>0$2.OZ!$1<@$Z>		user@site.OZ

#	At this point we look for names of the form
#	user@site and see if we can intuit a domain for
#	"site".  If so, we append the domain and try all over again.
R$+<@$*$=S>		$@$>0$1<@$2$3.$D>		Local host
R$+<@$*$=W>		$@$>0$1<@$2$3.UUCP>		(local) UUCP host
R$+<@$*$=C>		$@$>0$1<@$2$3.CSNET>		CSnet host
R$+<@$*$=A>		$@$>0$1<@$2$3.ARPA>		Arpanet host
R$+<@$*$=M>		$@$>0$1<@$2$3.MAILNET>		Mailnet host
R$+<@$*$=X>		$@$>0$1<@$2$3.UUCP>		(other) UUCP host
R$+<@$*$=B>		$@$>0$1<@$2$3.BITNET>		BITNET host
R$+<@$*$=E>		$@$>0$1<@$3.DEC>		DEC E-Net host

#	Error on any names with a network in them here since we couldn't
#	figure out where to send them.
R$*<@$+>$*		$#error$:Unknown host or domain in address

# remaining names are local (since they aren't on any of our networks)
R$+			$#local$:$1			everything else

########################################
###  Host dependent address cleanup  ###
########################################

S8
R$*$=U!$+@$+		$3@$4				drop uucp forward


include(uumail.m4)
include(pmdfm.m4)
include(etherm.m4)
