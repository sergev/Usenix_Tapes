############################################################
############################################################
#####
#####		PMDF Phonenet Channel Mailer specification
#####
#####	$Header: pmdfm.m4,v 5.1 85/10/13 20:46:01 spaf Release $
#####
############################################################
############################################################

Mpmdf,	P=/usr/local/lib/pmdf/pmdf-submit,	F=mDsFSn,	S=17, R=17,
	M=65535, A=pmdf-submit -f $g $u
#
#	Notice that the PMDF mailer DOES NOT USE the host field. We
#	set this host field to "CSNET-RELAY" in all instances where
#	we call the PMDF mailer so as to be able to send one copy
#	of a letter with a number of recipients.
#

S17

# pass <route-addr>'s through
R<@$+>$*		$@<@$1>$2			resolve <route-addr>

# map colons to dots everywhere.....
R$*:$*			$1.$2				map colons to dots

# handle the simple case....
R$+<@$+.$=K>		$@$>18$1<@$2.$3>		user@host.ARPA
R$+<@$-.CSNET>		$@$1<@$2.CSNET>			user@host.CSNET

R$+<@LOCAL>		$@$1<@$R.CSNET>			local names
R$+<@$+.LOCAL>		$@$1%$2<@$R.CSNET>		local notes
R$+<@$*$=S>		$@$1%$2$3<@$R.CSNET>		more local hosts

# handle other external cases
R$+<@$=X>		$@$1<@$2.UUCP>
R$+<@$->		$@$1<@$2>	
R$+<@$+.$-.$=T>		$@$1%$2<@$3.$4>			approximate something
R$+<@[$+]>		$@$1<@[$2]>			already ok

# convert remaining addresses to old format and externalize appropriately
#  We try to do nifty things to uucp addresses first
R$+<@$-.UUCP>		$2!$1
R$+!$+!$+		$2!$3
R$+!$+			$@$2@$1.UUCP

R$-:$+			$@$1.$2<@$A>			convert berk hosts
R$+<@$+>		$@$1%$2<@$A>			pessmize
R$+			$:$1<@$R.CSNET>			tack on our hostname
R$+%$=A<@$A>		$1<@$2>				strip out unneeded relay


S18

R$*<$+>$*		$@$1<$%2>$3
