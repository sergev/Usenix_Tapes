
/***********************************************************
 ***********************************************************
 ****
 ****	RULESET ZERO PREAMBLE
 ****
 ****	The beginning of ruleset zero is constant through all
 ****	configurations.
 ****
 ****	$Id: zerobase.cpp,v 1.4 86/04/04 14:31:36 kcs Exp $
 ****
 ***********************************************************
 ***********************************************************/

ruleset BASE_RW {

	/* first make canonical */

	if ( anypath < anypath > anypath )
		retry ( $1$2$3);			/* defocus */
	if ( path )
		next ( NAME_CANON ( $1 ) );		/* make canonical */

	/* handle special cases..... */

	if ( @ )
		resolve ( mailer (local),
			  user ("MAILER-DAEMON") );	/* handle <> form */
	if ( anypath < @[path] > anypath )
		resolve ( mailer (tcp),
			  host (hostnum ($2)),
			  user ("$1@[$2]$3") );		/* numeric internet spec */

	/* arrange for local names to be fully qualified */

	/***
	if ( anypath < anypath campushost > anypath )
		retry ( $1<$2$3".LOCAL">$4 );		/* user@campushost */
	/***
	if ( anypath < anypath berkhost > anypath )
		retry ( $1<$2$3".LOCAL">$4 );		/* user@berkhost */
	/***
	if ( anypath < path ".ARPA.LOCAL" > anypath )
		retry ( $1<$2".ARPA">$3 );		/* because ARPA is a host */

	/* delete local domain info -- this is a (temporary?) kludge */

	if ( anypath < anypath "." $localname > anypath )
		retry ( $1<$2>$3 );			/* strip local domain */
	if ( anypath < anypath "." localdomain > anypath )
		retry ( $1<$2>$4 );			/* strip local subdomains */

	/* now delete (more) local info */

	if ( anypath < anypath this_host "." topdomain_id > anypath )
		retry ( $1<$2>$5 );			/* this_host.LOCAL */
	if ( anypath < anypath this_host > anypath )
		retry ( $1<$2>$4 );			/* this_host */
	if ( anypath < anypath "." > anypath )
		retry ( $1<$2>$3 );			/* drop trailing dot */
	if ( < @ > : anypath )
		return ( BASE_RW ( $1 ) );		/* retry after route strip */
	if ( anypath < @ > )
		return ( BASE_RW ( $1 ) );		/* strip null trash and retry */


/********************************
 * End of ruleset zero preamble * 
 ********************************/
