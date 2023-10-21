############################################################
#
#  General configuration information
#
#  This information is basically just "boiler-plate"; it must be
#  there, but is essentially constant.
#
#  Information in this file should be independent of location --
#  i.e., although there are some policy decisions made, they are
#  not specific to Gatech per se.
#
#  $Header: base.m4,v 5.6 85/10/26 19:54:34 spaf Release $
#
############################################################

include(version.m4)

##########################
###   Special macros   ###
##########################

# my name
DnMAILER-DAEMON
# UNIX header format
DlFrom $g  $d
# delimiter (operator) characters
Do.:%@!^=/[]
# format of a total name
Dq$?x$x $.<$g>
# SMTP login message
De$j Sendmail $v/$V ready at $b

###################
###   Options   ###
###################

# location of alias file
OA/usr/lib/aliases
# wait for aliases to be up-to-date, and create them if need be
Oa
OD
# default delivery mode (deliver in background)
Odbackground
# (don't) connect to "expensive" mailers
Oc
# temporary file mode
OF0600
# default GID
Og1
# location of help file
OH/usr/lib/sendmail.hf
# log level
OL2
# default messages to old style
Oo
# queue directory
OQ/usr/spool/mqueue
# read timeout -- violates protocols
Or2h
# status file
OS/usr/lib/sendmail.st
# queue up everything before starting transmission
Os
# Queue when we're busy (x) and refuse SMTP when really busy (X)
Ox15
OX20
# default timeout interval
OT5d
# time zone names (V6 only)
OtEST,EDT
# default UID
Ou1
# wizard's password
OWa/FjIfuGKXyc2

###############################
###   Message precedences   ###
###############################

Pfirst-class=0
Pspecial-delivery=100
Pjunk=-100

#########################
###   Trusted users   ###
#########################

Troot
Tdaemon
Tspaf
Tuucp
Tnetwork

#############################
###   Format of headers   ###
#############################

H?P?Return-Path: <$g>
HReceived: $?sfrom $s $.by $j $?rwith $r $.($v/$V)
	id $i; $b
H?D?Resent-Date: $a
H?D?Date: $a
H?F?Resent-From: $q
H?F?From: $q
H?x?Full-Name: $x
HSubject:
HPosted-Date: $a
H?l?Received-Date: $b
# H?M?Resent-Message-Id: <$t.$i@$j>
H?M?Message-Id: <$t.$i@$j>

###########################
###   Rewriting rules   ###
###########################


################################
#  Sender Field Pre-rewriting  #
################################
S1

###################################
#  Recipient Field Pre-rewriting  #
###################################
S2

#################################
#  Final Output Post-rewriting  #
#################################
S4

R@			$@				handle <> error addr

# externalize local domain info
R$*<$*LOCAL>$*		$1<$2$D>$3			change local info
R$*<$+>$*		$1$2$3				defocus
R$*$=S:$*		$1$2!$3
R@$+:$+:$+		$@@$1,$2:$3			<route-addr> canonical

# delete duplicate local names -- mostly for arpaproto.mc
R$+%$=w@$=w		$1@$3				u%UCB@UCB => u@UCB
R$+%$=w@$=w.$=D		$1@$3.$D			u%UCB@UCB => u@UCB

# clean up uucp path expressions (some)
R$*!$*@$*.UUCP		$3!$1!$2

###########################
#  Name Canonicalization  #
###########################
S3

# handle "from:<>" special case
R<>			$@@				turn into magic token
R$*$=S:$=S$*		$1$3$4
R$*$=S!$=S$*		$1$3$4

# basic textual canonicalization
R$*<$+>$*		$2				basic RFC821/822 parsing
R$+ at $+		$1@$2				"at" -> "@" for RFC 822
R$*<$*>$*		$1$2$3				in case recursive

# make sure <@a,@b,@c:user@d> syntax is easy to parse -- undone later
R@$+,$+			@$1:$2				change all "," to ":"

# localize and dispose of domain-based addresses
R@$+:$+			$@$>6<@$1>:$2			handle <route-addr>

# more miscellaneous cleanup
R$+			$:$>8$1				host dependent cleanup
R$+:$*;@$+		$@$1:$2;@$3			list syntax

# Handle special case of received via uucp
R$+!$*			$:<$&r>$1!$2			check arriving protocol
R$+^$*			$:<$&r>$1!$2			both syntaxes
R<UUCP>$*@$-.UUCP	$@$>6$1<@$2.UUCP>		...if the second time
R<UUCP>$+!$*		$@$>6$2<@$1.UUCP>		if via UUCP, resolve
R<$*>$*			$2				undo kludge
R$+@$+			$:$1<@$2>			focus on domain
R$+<$+@$+>		$1$2<@$3>			move gaze right
R$+<@$+>		$@$>6$1<@$2>			already canonical

# convert old-style addresses to a domain-based address
R$+%$+			$:$1<@$2>			user%host
R$+<@$+%$+>		$1%$2<@$3>			fix user%host1%host2
R$+<@$+>		$@$>6$1<@$2>			leave

R$-:$+			$@$>6$2<@$1>			host:user
R$-.$+			$@$>6$2<@$1>			host.user
R$+^$+			$1!$2				convert ^ to !
R$-!$+			$@$>6$2<@$1.UUCP>		resolve uucp names
R$-=$+			$@$>6$2<@$1.BITNET>		resolve bitnet names
