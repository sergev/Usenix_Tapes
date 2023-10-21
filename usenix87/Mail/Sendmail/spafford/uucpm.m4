############################################################
############################################################
#####
#####  	UUCP Mailer specification
#####
#####  	$Header: uucpm.m4,v 5.1 85/10/13 20:46:14 spaf Release $
#####
############################################################
############################################################

Muucp,	P=/usr/bin/uux, F=sDFMuU, S=13, R=23, M=65535,
	A=uux - -L -a$f -gC $h!rmail ($u)

S13
R$+			$:$>5$1				convert to old style
R$=w!$+			$2				strip local name
R$*<@$->$*		$1<@$2.UUCP>$3			resolve abbreviations
R$*<@$*.UUCP>$*		$:$>5$1<@$2.UUCP>$3
R$+			$:$U!$1				stick on our host name
R$=w!$=R$+		$:$2$3
R$*$=w!$=w$*		$1$U$4

S23
R$+			$:$>5$1				convert to old style
R$*<@$=S>$*		$1<@$2.$D.UUCP>$3		resolve abbreviations
R$*<@$R.$D.UUCP>$*	$1<@$2.UUCP>$3


S5
R$+<@$-.LOCAL>		$1%$2				u@h.LOCAL => u%h
R$+<@$-.UUCP>		$2!$1				u@host.UUCP => host!u
R$+@$+.$=T		$1@$2				u@host.ARPA => u@host
