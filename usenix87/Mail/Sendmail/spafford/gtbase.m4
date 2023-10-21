#################################################
#
#  General configuration information and so on
#  Specific to GaTech sites
#
#  $Header: gtbase.m4,v 5.2 85/10/26 18:53:29 spaf Release $
#
#################################################

######################
#   General Macros   #
######################


# local domain names
DDGTNET
CDgtnet gatech git gt

# My "top-level" domain (seen on Gatech)
DTCSNET

# major relay host
DRgatech
CRgatech GATECH GATech Gatech GaTech GAtech

# and forwarding host for ether mail
DFgatech

# my official hostname
Dj$w.$D

# known top-level domains
CTARPA UUCP BITNET CSNET MAILNET DEC EDU GOV MIL COM ORG NET OZ UK

# known Internet domains (we send on as if ARPA)
CKARPA EDU GOV MIL COM ORG NET UK

# UUCP hosts that we talk to
FW/usr/lib/mail/uucp.local


include(base.m4)

#######################
#   Rewriting rules   #
#######################

##### special local conversions
S6
# Recognize "old" syntax mistakes, like UUCP specifications of Ethernet
#	hosts
R$*<@$=W>$*		$1<@$2.UUCP>$3
R$*<@$=S.UUCP>		$1<@$2.$D>			uucp-isms
#
R$*<@$*$=D>$*		$1<@$2LOCAL>$4			convert local domain
R$*<@$=S>$*		$1<@$2.LOCAL>$3			user@localhost
R$*<@$+$=S>$*		$1<@$2$3.LOCAL>$4		user@host.subdomain
R$*<@$*$=D.$=T>$*	$1<@$2LOCAL>$5			catch "gtnet.csnet"
R$*<@LOCAL>		$1				degenerate case

R$+%$+<@$R.LOCAL>	$1<@$2.LOCAL>			hacks for % syntax
R$+%$=S<@$=S.LOCAL>	$1<@$2.LOCAL>			relayed internally

R$*<@$+.$=D.$=D>$*	$1<@$2.$3>$5			make gtnet top level

include(localm.m4)

include(zerobase.m4)

