
/************************************************
 ************************************************
 ****
 ****	General configuration information
 ****
 ****	$Id: puccbase.cpp,v 1.4 86/04/04 14:25:46 kcs Exp $
 ****
 ************************************************
 ************************************************/

/********************
 *  General Macros  *
 ********************/

macro
	localname = "PURDUE.EDU"; 		/* local domain names */
	relay_host = "pucc-h";			/* berknet relay host */
	m_oname = "${m_sitename}";		/* my official hostname (sort of */

class
	localname = { "PURDUE.EDU", PURDUE, CC, Purdue-CS, CS, ECN };
	topdomain = { LOCAL, ARPA, UUCP, BITNET, CSNET };	/* known top-level domains */

#include "localhosts.cpp"
#include "berkhosts.cpp"
#include "base.cpp"

/*********************
 *  Rewriting rules  *
 *********************/

/*** special local conversions ***/
S6
# R$*<@$*$=D>$*		$1<@$2LOCAL>$4			convert local domain
# R$*<@$*$=D.ARPA>$*	$1<@$2LOCAL>$4
R$*<@$+.$=T.$=T>$*	$1<@$2.$3>$5			delete dup top level
R$*<@$*$=S.UUCP>$*	$1<@$2$3>$4			local UUCP direct
R$*<@$*$=Z.UUCP>$*	$1<@$2$3>$4			local UUCP direct
R$*<@$=Y$*>$*		$1<@cs$3>$4			cs-host => cs kludge


#include "localm.cpp"
