############################################################
############################################################
#####
#####  	Local and Program Mailer specification
#####
#####  	$Header: localm.m4,v 5.1 85/10/13 20:45:58 spaf Release $
#####
############################################################
############################################################

Mlocal,	P=/bin/mail, F=rlsDFMmn, S=10, A=mail -d $u
Mprog,	P=/bin/sh,   F=lsDFMe,   S=10, A=sh -c $u

S10
R@			MAILER-DAEMON			errors to mailer-daemon
