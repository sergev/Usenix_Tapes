/***********************************************************
 ***********************************************************
 ****
 ****	SENDMAIL CONFIGURATION FILE
 ****
 ****	For PUCC-J
 ****
 ****	$Id: pucc-j.mc,v 1.2 86/04/04 14:20:57 kcs Exp $
 ****
 ***********************************************************
 ***********************************************************/



/***********************************************************
 **	local info
 ***********************************************************/

macro
	m_sitename = "j.cc.purdue.edu"; 	/* internet hostname */
	arpa_name  = "asc";			/* our arpanet name */

class
	m_sitename = { "j.cc.purdue.edu", pucc-j, puccj, j, pucc, purdue-asc, 
		        purdue-205-gw, purdue-asc-tn, asc, "asc.purdue.edu", 
		       "asc.cc.purdue.edu", 205-gw, asc-tn };

	arpa_name = { "asc.purdue.edu", "asc.cc.purdue.edu", asc, purdue-asc,
		       purdue-205-gw };

#include "phonenethosts.cpp"

#include "puccbase.cpp"

#include "zerobase.cpp"

/**********************************************
 **  Machine dependent part of ruleset zero  **
 **********************************************/

	/* send berknet names through relay */

	if ( < @berkhost > : path )
		resolve ( mailer ( pcl ),
			  host ( $relay_host ),
			  user ( $2<@$1> ) );		/* @berkhost: ... */

	if ( anypath < @anypath berkhost > anypath )
		resolve ( mailer ( pcl ),
			  host ( $relay_host ),
			  user ( $1<@$2$3>$4 ) );	/* user@berknethost */

	/* resolve campus names */

	if ( anypath < @anypath campushost > anypath )
		resolve ( mailer ( pcl ),
			  host ( $3 ),
			  user ( $1<@$2$3>$4 ) );	/* user@campushost */

	/* send csnet names through relay */

	if ( anypath < @phonehost > anypath )
		resolve ( mailer ( tcp ),
			  host ( "CSNet-Relay" ),
			  user ( $1%$2<@"CSNet-Relay">$3 ) );

	if ( anypath < @phonehost ".ARPA" > anypath )
		resolve ( mailer ( tcp ),
			  host ( "CSNet-Relay" ),
			  user ( $1%$2<@"CSNet-Relay">$3 ) );

	if ( anypath < @path ".CSNET" > anypath )
		resolve ( mailer ( tcp ),
			  host ( "CSNet-Relay" ),
			  user ( $1%$2<@"CSNet-Relay">$3 ) );

	/* we don't do uucp */

	if ( anypath < @anypath ".UUCP" > anypath )
		resolve ( mailer ( error ),
			  user ( "Non-Local UUCP Mail Unavailable" ) );

	/* we don't do bitnet (yet) */

	if ( anypath < @anypath ".BITNET" > anypath )
			resolve ( mailer ( error ),
				  user ( "Bitnet Mail Unavailable At This Time" ) );

	/* other non-local names to the arpanet */

	if ( anypath < @anypath > anypath )
		resolve ( mailer ( tcp ),
			  host ( $2 ),
			  user ( $1<@$2>$3 ) );		/* user@arpahost */

	/* remaining names must be local */

	if ( path )
		resolve ( mailer ( local ),
			  user ( $1 ) );		/* everything else */

/**************************************
 **  Host dependent address cleanup  **
 **************************************/

ruleset HOSTDEP_RW {

	if ( anypath uucphost!usr@hostpath )
		retry ( $3@$4 );			/* drop uucp forward */

}

#include "pclm.cpp"
#include "tcpm.cpp"
