/***********************************************************
 ***********************************************************
 ****
 ****	SENDMAIL CONFIGURATION FILE
 ****
 ****	For PUCC-H
 ****
 ****	$Id: pucc-h.mc,v 1.2 86/04/04 14:18:03 kcs Exp $
 ****
 ***********************************************************
 ***********************************************************/


/***********************************************************
 **	local info
 ***********************************************************/

macro
	m_sitename = "h.cc.purdue.edu";		/* internet hostname */
	berkname   = "h.cc.purdue.edu";		/* berknet hostname  */

class
	m_sitename = { "h.cc.purdue.edu", pucc-h, pucch, h };


#include "puccbase.cpp"


options
	/***
	o_slog = "1";		/* override logging level in base.cpp */


#include "zerobase.cpp"


/***********************************************
 **  Machine dependent part of ruleset zero   **
 ***********************************************/

	/* resolve berknet names */

	if ( < @berkhost > : path )
		resolve ( mailer ( berk ),
			  host ( $1 ),
			  user ( $2 ) );		/* @berkhost: ... */

	if ( usr < @berkhost > )
		resolve ( mailer ( berk ),
			  host ( $2 ),
			  user ( $1 ) );		/* user@berknethost */

	/* we don't do uucp */

	if ( anypath < @anypath ".UUCP" > anypath )
		resolve ( mailer ( error ),
			  user ( "Non-Local UUCP Mail Unavailable" ) );

	/* resolve campus names (actually, all non-local names) */

	if ( anypath < @anypath hostname > anypath )
		resolve ( mailer ( pcl ),
			  host ( $3 ),
			  user ( $1<@$2$3>$4 ) );	/* user@campushost */

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
#include "berkm.cpp"
