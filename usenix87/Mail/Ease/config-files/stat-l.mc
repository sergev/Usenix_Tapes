############################################################
############################################################
#####
#####	SENDMAIL CONFIGURATION FILE
#####
#####	For STAT-L
#####
#####	$Id: stat-l.mc,v 1.2 86/04/04 14:27:47 kcs Exp $
#####
############################################################
############################################################



############################################################
###	local info
############################################################

# internet hostname
Dwstat-l
Cwstat-l statl l stat

include(puccbase.cpp)

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
