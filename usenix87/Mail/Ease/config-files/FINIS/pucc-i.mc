/***********************************************************
 ***********************************************************
 ****
 ****	SENDMAIL CONFIGURATION FILE
 ****
 ****	For PUCC-I
 ****
 ****	$Id: pucc-i.mc,v 1.2 86/04/04 14:19:21 kcs Exp $
 ****
 ***********************************************************
 ***********************************************************/



/***********************************************************
 **	local info
 ***********************************************************/

macro
	m_sitename = "i.cc.purdue.edu"; 	/* internet hostname */

class
	m_sitename = { "i.cc.purdue.edu", pucc-i, pucci, i };

#include "puccbase.cpp"

options
	/***
	o_slog = "1"; 		/* override logging level in base.cpp */

#include "zerobase.cpp"

/**********************************************
 **  Machine dependent part of ruleset zero  **
 **********************************************/

	/* send berknet names through relay */

	if ( < @berkhost > : path )
		resolve ( mailer ( pcl ),
		  	  host ( $relay-host ),
		  	  user ( $2<@$1> ) );		/* @berkhost: ... */

	if ( anypath < @anypath berkhost > anypath )
		resolve ( mailer ( pcl ),
			  host ( $relay-host ),
			  user ( $1<@$2$3>$4 ) );	/* user@berknethost */

	/* we don't do uucp */

	if ( anypath < @anypath ".UUCP" > anypath )
		resolve ( mailer ( error ),
			  user ( "Non-Local UUCP Mail Unavailable" ) );

	/* resolve campus names (actually, all non-local names) */

	if ( anypath < @anypath hostname > anypath )
		resolve ( mailer ( pcl ),
			  host ( $3 ),
			  user ( $1<@$2$3>$4 ) );	/* user@campushost */

	/* remaining names must be local */

	if ( path )
		resolve ( mailer ( local ),
			  user ( $1 ) );		/* everything else */


/**************************************
 **  Host dependent address cleanup  **
 **************************************/

ruleset HOSTDEP_RW {

	if ( anypath uucphost!usr@path )
		retry ( $3@$4 );			/* drop uucp forward */

	if ( anypath campushost ".ARPA" anypath )
		retry ( $1$2$3 );			/* drop ".ARPA" */

}

#include "pclm.cpp"
