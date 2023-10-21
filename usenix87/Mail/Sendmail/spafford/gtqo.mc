############################################################
############################################################
#####
#####		SENDMAIL CONFIGURATION FILE FOR GTQO
#####
#####	$Header: gtqo.mc,v 5.1 85/10/13 20:38:28 spaf Release $
#####
############################################################
############################################################



############################################################
###	local info
############################################################

# internet hostname
Cwgtqo

# UUCP name
DUgtqo
CUgtqo gt-qo

# Ethernet stuff
CS gtss gtqo gt-ss gt-qo

include(gtbase.m4)
DRgtss
CR
CRgtss gt-ss
DFgtss
include(short3.m4)
