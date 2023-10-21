/***********************************************************
 ***********************************************************
 ****
 ****	SENDMAIL CONFIGURATION FILE
 ****
 ****	For PUCC-S
 ****
 ****	$Id: pucc-s.mc,v 1.1 86/03/31 11:16:37 kcs Exp Locker: kcs $
 ****
 ***********************************************************
 ***********************************************************/



/***********************************************************
 **	local info
 ***********************************************************/

/* internet hostname */
Dws.cc.purdue.edu
Cws.cc.purdue.edu pucc-s puccs s

#include "puccbase.cpp"

/* override logging level in base.cpp */
# OL1

include(zerobase.cpp)

################################################
###  Machine dependent part of ruleset zero  ###
################################################

# send berknet names through relay
R<@$=Z>:$+		$#pcl$@$R$:$2<@$1>		@berkhost: ...
R$*<@$*$=Z>$*		$#pcl$@$R$:$1<@$2$3>$4		user@berknethost

# we don't do uucp 
R$*<@$*.UUCP>$*		$#error$:Non-Local UUCP Mail Unavailable

# resolve campus names (actually, all non-local names)
R$*<@$*$->$*		$#pcl$@$3$:$1<@$2$3>$4		user@campushost

# remaining names must be local
R$+			$#local$:$1			everything else

########################################
###  Host dependent address cleanup  ###
########################################

S8
R$*$=U!$+@$+		$3@$4				drop uucp forward
R$*$=S.ARPA$*		$1$2$3				drop ".ARPA"


include(pclm.cpp)
