/***********************************************************
 ***********************************************************
 ****
 ****	Provide Backward Compatibility
 ****
 ****	$Id: compat.cpp,v 1.4 85/11/24 22:46:32 acu Exp $
 ****
 ***********************************************************
 ***********************************************************/

#define cppCOMPAT 4.2

field
	berkhost: match (1) in berkhosts;

/***************************************************
 * General code to convert back to old style names *
 ***************************************************/

ruleset OLDSTYLE_RW {

	if ( usr < @berkhost > )
		return ( $2:$1 );			/* u@bhost => h:u */

	if ( usr < @hostname ".UUCP" > )
		retry ( $2!$1 );			/* u@host.UUCP => host!u */

	if ( usr @ hostpath ".ARPA" )
		retry ( $1@$2 );			/* u@host.ARPA => u@host */

}
