################################################
###  Machine dependent part of ruleset zero  
###
###  Short version for non-ICS machines not on 
###  common ethernet. (e.g., gt-cmmsr)
###
###  $Header: short2.m4,v 5.1 85/10/13 20:46:05 spaf Release $
################################################

# resolve names that we can handle locally
R<@$=W.UUCP>$+		$#uucp$@$1$:$2			@host.UUCP: ...
R$+<@$=W.UUCP>		$#uucp$@$2$:$1			user@host.UUCP

# other non-local names will be kicked upstairs
R$*<@$+>$*		$#uucp$@$F$:$1<@$2>$3		user@some.where

# remaining names must be local
R$+			$#local$:$1			everything else

include(uucpm.m4)
