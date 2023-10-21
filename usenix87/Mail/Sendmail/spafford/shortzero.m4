################################################
###  Machine dependent part of ruleset zero  
###
###  Short version for ICS machines on common
###  ethernet. (NOT "gatech")
###
###  $Header: shortzero.m4,v 5.1 85/10/13 20:46:11 spaf Release $
################################################

# resolve names that we can handle locally
R<@$=W.UUCP>$+		$#uucp$@$1$:$2			@host.UUCP: ...
R$+<@$=W.UUCP>		$#uucp$@$2$:$1			user@host.UUCP

# resolve names that can go via the ethernet
R$*<@$*$=S.LOCAL>$*	$#ether$@$3$:$1<@$2$3.$D>$4	user@etherhost

# other non-local names will be kicked upstairs
R$*<@$+>$*		$#ether$@$F$:$1<@$2>$3		user@some.where

# remaining names must be local
R$+			$#local$:$1			everything else

include(uucpm.m4)
include(etherm.m4)
