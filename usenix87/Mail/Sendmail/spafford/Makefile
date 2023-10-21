######################################################################
#
#	Makefile for Sendmail GaTech configuration files
#
#		$Header: Makefile,v 5.6 85/10/26 18:59:46 spaf Release $
#
######################################################################


M4	= base.m4 csether.m4 etherm.m4 gtbase.m4 localm.m4 shortzero.m4 \
	  uucpm.m4 uumail.m4 version.m4 zerobase.m4 pmdfm.m4 short2.m4 \
	  short3.m4 
MC	= gatech.mc stratus.mc nimbus.mc gitpyr.mc cirrus.mc gt-cmmsr.mc \
	  gtss.mc gtqo.mc
SRCS=	$(MC) $(M4)

CSALL	= gatech.cf gitpyr.cf stratus.cf nimbus.cf cirrus.cf
ALL	= $(CSALL) gt-cmmsr.cf gtss.cf gtqo.cf

GET=	co

.SUFFIXES: .mc .cf

.mc.cf:
	m4 $*.mc > $*.cf

all: $(ALL)

shar:   
	co -q -sRelease $(SRCS) Makefile MANIFEST README KEY PATCHES \
	PATCHES2 PATCH3 uumail.c overview.ms Files
	shar -v -pX `sed -n '5,$$s/  .*//p' MANIFEST | tr '\12' '\40'` > sendmail.shar
	compress -c < sendmail.shar > sendmail.shar.Z

cs:  $(CSALL)


############################
#   special dependencies   #
############################

# all computer science department hosts....
$(CSALL): csether.m4

# pessimize -- basic dependencies
$(ALL): base.m4 localm.m4 uucpm.m4 version.m4 zerobase.m4 gtbase.m4

# hosts using MMDF/PMDF
gatech.cf: pmdfm.m4

# Hosts using Ethernet/SMTP
$(CSALL): etherm.m4 
gtss.cf gtqo.cf: etherm.m4

# Hosts using optimized uucp mailer
gatech.cf: uumail.m4

# Hosts using short definition
stratus.cf: shortzero.m4
nimbus.cf: shortzero.m4
cirrus.cf: shortzero.m4
gitpyr.cf: shortzero.m4

# Other cases
gt-cmmsr.cf: short2.m4
gtss.cf gtqo.cf: short3.m4

# Specifics on hosts
gatech.cf: gatech.mc
gitpyr.cf: gitpyr.mc
stratus.cf: stratus.mc
nimbus.cf: nimbus.mc
cirrus.cf: cirrus.mc
gt-cmmsr.cf: gt-cmmsr.mc
gtss.cf: gtss.mc
gtqo.cf: gtqo.mc

#####################
#   administrivia   #
#####################

sources: $(SRCS)

$(SRCS):
	$(GET) $@

README MANIFEST overview.ms KEY Makefile Files:
	$(GET) $@

clean:
	rm -f *.cf a.out core make.out Make.errs

