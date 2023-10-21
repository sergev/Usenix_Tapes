######################################################################
#
#	Makefile for Sendmail PUCC configuration files
#
#	$Id: Makefile,v 1.4 86/04/04 14:16:17 kcs Exp $
#
######################################################################


PUCCSRCS=pucc-h.mc \
	pucc-i.mc \
	pucc-j.mc \
	pucc-k.mc \
	stat-l.mc \
	pucc-n.mc \
	pucc-s.mc

GENSRCS=base.cpp \
	berkhosts.cpp \
	berkm.cpp \
	compat.cpp \
	localhosts.cpp \
	localm.cpp \
	pclm.cpp \
	puccbase.cpp \
	tcpm.cpp \
	version.cpp \
	zerobase.cpp

SRCS=	$(GENSRCS) $(PUCCSRCS)

PUCCALL=pucc-h.cf \
	pucc-i.cf \
	pucc-j.cf \
	pucc-k.cf \
	stat-l.cf \
	pucc-n.cf \
	pucc-s.cf

ALL=	$(PUCCALL)


.SUFFIXES: .mc .cf

.mc.cf:
	/lib/cpp $*.mc > $*.cf

all: $(ALL)

############################
#   special dependencies   #
############################

# pessimize -- basic dependencies
$(ALL): base.cpp berkhosts.cpp localm.cpp localhosts.cpp pclm.cpp puccbase.cpp \
	version.cpp zerobase.cpp

# hosts on berknet
pucc-h.cf: berkm.cpp

# hosts on the arpanet
pucc-j.cf: tcpm.cpp

# hosts on csnet
pucc-j.cf: phonenethosts.cpp

# mailers requiring backwards compatibility
berkm.cpp tcpm.cpp: compat.cpp

#####################
#   administrivia   #
#####################

sources: $(SRCS)

clean:
	rm -f $(ALL) a.out core errs
