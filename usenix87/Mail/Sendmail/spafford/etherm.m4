############################################################
############################################################
#####
#####  	Ethernet Mailer specification
#####
#####  	$Header: etherm.m4,v 5.1 85/10/13 20:45:52 spaf Release $
#####
############################################################
############################################################

Mether,	P=[IPC], F=msDFIMuCX, S=11, A=IPC $h

S11
R$*<@$+>$*		$@$1<@$2>$3			already ok
R$+			$@$1<@$w.LOCAL>			tack on our hostname
