/***********************************************************
 ***********************************************************
 ****
 ****	Berknet Mailer specification
 ****
 ****	$Id: berkm.cpp,v 1.4 85/07/10 22:32:07 acu Exp $
 ****
 ***********************************************************
 ***********************************************************/

#ifdef cppCOMPAT
#include "compat.cpp"
#endif

mailer
	berk {
		Path	= "/usr/net/bin/sendberkmail",
		Flags	= { f_ffrom, f_strip, f_date, f_from, f_mesg, f_addrw };
		Sender  = BERKSEND_RW;
		Recipient = BERKREC_RW;
		Argv	= "sendberkmail -m ${m_rhost} -h ${m_hops} -t ${m_ruser}";
		Maxsize	= "25000"
	};

ruleset BERKSEND_RW {

	if ( path )
		next ( OLDSTYLE_RW ( $1 ) );		/* convert to old style */
	if ( hostname:usr )
		return ( $1:$2);			/* old berknet as is */

	if ( path < @path > )
		return ( $1<@$2> );			/* don't modify arpanet */

	if ( hostname!usr )
		return ( $1!$2 );			/* don't modify uucp */

	if ( < @path > )
		return ( <@$1> );			/* don't modify <routeaddr> */

	if ( path )
		return ( $m_shostname:$1 );		/* make others relative */

}


ruleset BERKREC_RW {

	if ( path )
		next ( OLDSTYLE_RW ( $1 ) );		/* convert to old style */

}
