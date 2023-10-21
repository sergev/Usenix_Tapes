############################################################
############################################################
#####
#####  	UUCP Mailer specification
#####   This is for the rerouting mailer on Gatech
#####
#####  	$Header: uumail.m4,v 5.2 85/10/26 18:40:33 spaf Release $
#####
############################################################
############################################################

# use fancy path expanding UUCP frontend.
Muucp,	P=/usr/lib/mail/uumail, F=sCDFMSU, S=13, R=23, M=65535,
	A=uumail -f $g $h!$u

S13
R$+			$:$>5$1				convert to old style
R$=w!$+			$2				strip local name
R$*<@$->$*		$1<@$2.UUCP>$3			resolve abbreviations
R$*<@$*.UUCP>$*		$:$>5$1<@$2.UUCP>$3
R$+			$:$U!$1				stick on our host name
R$=w!$=R$+		$:$2$3
R$*$=w!$=w$*		$1$U$4

S23
R$*<@$-.LOCAL>$*	$1<@$2.UUCP>$3
R$*<@$=S>$*		$1<@$2.UUCP>$3			resolve abbreviations
R$*<@$R.$D.UUCP>$*	$1<@$2.UUCP>$3
R$+!$+!$+<@$*.UUCP>$*	$2!$3<@$4.UUCP>$5		strip leading sites
R$+!$+<@$*.UUCP>$*	$:$2<@$1.UUCP>$4		put in right sitename


S5
R$+<@$-.LOCAL>		$2!$1				u@h.LOCAL => h!u
R$+<@$-.UUCP>		$2!$1				u@host.UUCP => host!u
