#! /bin/make -f
# MAKEFILE FOR THE PATHALIAS DATABASE
#
#	@(#)Makefile	1.9	9/25/86
#	@(#) Ronald S. Karr <tron@SC.NSC.COM>
#	@(#) National Seiconductor, Sunnyvale
#
# keywords:
#	full.paths - make pathalias database with domain and USENET maps.
#	domain.paths - make pathalias database with only domain maps.
#	local.paths - make the local domain paths.
#	extern.paths - make the domain-external paths.
#	dead.script - make sed script out of dead file.
#	hostmapfile - install our current entry in the USENET maps.
#	stripdom - make the stripdom program.
#	pathmerge - make the pathmerge program.

# HOST INFORMATION
#	who we are
HOST		= oursite
#
#	the USENET map file containing our map entry
#	For example:  nsc is currently in u.usa.ca.4.
#	This can change, so be prepared to make sure it is
#	correct every once in a while.
#
#	If you don't know, leave this blank
HOSTMAPFILE	= u.usa.ca.1
#
#	our own, current, map entry file
#	If HOSTMAPFILE is blank, leave HOSTFILE blank as well.
HOSTFILE	= oursite.samp
#
#	define this to be the site we forward mail to that
#	we cannot find a path to in our database.
#	Set it to nothing if we do not have a more intelligent
#	mail forwarding site.
#
#	The format should be:  forwarding-relay = hostname
#	If you hard-coded the mail forwarding site in the smail
#	defs.h program, don't worry about this one.
FORWARD		= 

# DOMAIN INFORMATION
#	List here domains that we wish to be able to abbreviate.
#	I.e., if you wished to be able to abbreviate the domain
#	.subdomain.domain to allow the use of only .subdomain,
#	define that here.
DOMAINS		= .domain.com .sub.domain.com .far.domain.com
#
#	Files to include for domain map.
DOMAINFILES	= domain.com.samp sub.domain.com.samp

# USENET MAP DATABASE
#	Make this the directory containing the compressed
#	USENET map files.
MAPS		= /usr/spool/uumaps

# EXTERNAL INFORATION
#	Files for generation of external paths
#	(not including USENET Map database)
EXTERNFILES	= local.map

# DESTINATION
#	Directory containing the paths file and DBM database.
PATHDIR		= /usr/lib/uucp

#	PATH variable for the shell
PATH		= .:/usr/local:/usr/local/bin:/usr/new:/usr/ucb:/usr/bin:/bin

SHELL		= /bin/sh

#	If you are including the USENET maps, make this full.paths
#	Otherwise, make this domain.paths.
all:		full.paths

# build paths file given full USENET and local domain information.
# The pathmerge seems to make mistakes when given more than two files,
# so we only give it two files at a time, here.
full.paths:	.full.paths
.full.paths:	stripdom pathmerge extern.paths local.paths forces
		sed	-e '/^#/d' \
			-e '/^[ 	]*$$/d' \
			-e 's/[ 	][ 	]*/	/' \
			< forces | sort | \
			pathmerge - local.paths | pathmerge - extern.paths | \
			stripdom ${DOMAINS} > ${PATHDIR}/.pa.temp
		makedb -o ${PATHDIR}/.pa.temp ${PATHDIR}/.pa.temp
		mv -f ${PATHDIR}/.pa.temp     ${PATHDIR}/paths
		-if test makedb != true; then \
			mv -f ${PATHDIR}/.pa.temp.dir ${PATHDIR}/paths.dir;\
			mv -f ${PATHDIR}/.pa.temp.pag ${PATHDIR}/paths.pag;\
		fi
		rm -f .domain.paths
		touch .full.paths

# build paths file only with domain information.
domain.paths:	.domain.paths
.domain.paths:	stripdom pathmerge local.paths forces
		sed	-e '/^#/d' \
			-e '/^[ 	]*$$/d' \
			-e 's/[ 	][ 	]*/	/' \
			< forces | sort | \
			pathmerge - local.paths | \
			stripdom ${DOMAINS} > ${PATHDIR}/.pa.temp
		makedb -o ${PATHDIR}/.pa.temp ${PATHDIR}/.pa.temp
		mv -f ${PATHDIR}/.pa.temp     ${PATHDIR}/paths
		mv -f ${PATHDIR}/.pa.temp.dir ${PATHDIR}/paths.dir
		mv -f ${PATHDIR}/.pa.temp.pag ${PATHDIR}/paths.pag
		rm -f .full.paths
		touch .domain.paths

# build a file containing paths for the local domain.
local.paths:	${DOMAINFILES}
		(	echo ${FORWARD}; \
			cat ${DOMAINFILES}; \
		) | pathalias -l ${HOST} -i 2>> ERRORS | sort > local.paths

# build a file containing paths for the USENET.
# the uuwhere stuff isn't critical, so ignore any errors from it.
extern.paths:	/DONE .hostmapfile ${EXTERNFILES} dead.script
		(	echo ${FORWARD}; \
			zcat ${MAPS}/u.*.Z; \
			cat ${EXTERNFILES}; \
		) | pathalias -l ${HOST} -i `cat dead.script` 2>> ERRORS | \
			sort > extern.paths
		-if test -f ${MAPS}/lib/uuwhere; then ${MAPS}/lib/uuwhere; fi

# The file DONE should exist if you use uuhosts -unbatch for news.
# but if it doesn't, then this will keep extern.paths happy.
/DONE:
		-touch /DONE

# Remove the comments here to enable the automatic editing of
# the map files to include an up-to-date copy of your map entry.
hostmapfile:	
.hostmapfile:	${MAPS}/${HOSTMAPFILE}.Z ${HOSTFILE}
#		-zcat ${MAPS}/${HOSTMAPFILE}.Z > ${MAPS}/.ho.temp
#		(	echo   "1;/^#N[ 	]*${HOSTFILE}$$/,//;/^#N/-2d"; \
#			echo	".-1r ${HOST}"; \
#			echo	"w"; \
#			echo	"q"; \
#		) | ed - ${MAPS}/.ho.temp
#		-compress ${MAPS}/.ho.temp
#		-mv -f ${MAPS}/.ho.temp.Z ${MAPS}/${HOSTMAPFILE}.Z
		touch .hostmapfile

# Create a file which can be cat'ed into the argument list for pathalias.
dead.script:	dead
		awk '/^[^# 	]/ { print "-d " $$1 }' < dead > dead.script

stripdom:	stripdom.c
		${CC} ${CFLAGS} -o stripdom stripdom.c
pathmerge:	pathmerge.c
		${CC} ${CFLAGS} -o pathmerge pathmerge.c
