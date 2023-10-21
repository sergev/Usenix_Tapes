
/***********************************************************
 ***********************************************************
 ****
 ****	Arpanet TCP Mailer specification
 ****
 ****	Messages processed by this specification are assumed to leave
 ****	the local domain -- hence, they must be canonical according to
 ****	RFC822 etc.
 ****
 ****	$Id: tcpm.cpp,v 1.5 86/04/04 14:29:16 kcs Exp $
 ****
 ***********************************************************
 ***********************************************************/

#ifndef cppCOMPAT
#include "compat.cpp"
#endif

mailer
	tcp { Path	= "[IPC]",
	      Flags	= { f_mult, f_date, f_from, f_mesg, f_upperu,
			    f_dot, f_llimit },
	      Sender	= MAP_RW,
	      Recipient = MAP_RW,
	      Argv	= "IPC ${m_rhost}",
	      Eol	= "\r\n",
	      Maxsize	= "100000"
	};


ruleset MAP_RW {

	/* pass <route-addr>'s through */

	/***
	if ( < @path > anypath )
		return ( <@canon($1)>$2 );		/* resolve <route-addr> */

	if ( < @path > anypath )
		return ( <@$1>$2 );			/* resolve <route-addr> */

	/* map colons to dots everywhere..... */

	if ( anypath : anypath )
		retry ( $1.$2 );			/* map colons to dots */

	/* handle the simple case.... */

	/***
	if ( path < @hostname ".ARPA" > )
		return ( $1<@canon($2".ARPA")> );	/* user@host.ARPA */

	if ( path < @hostname ".ARPA" > )
		return ( $1<@$2".ARPA"> );		/* user@host.ARPA */

	/***
	if ( path < @hostpath ".LOCAL" > )
# R$+<@$+.LOCAL>	$@$1%$2<@$A>			local hosts
# R$+<@$+.BITNET>	$@$1%$2.BITNET<@$A>		user@host.BITNET
# R$+<@$+.CSNET>	$@$1.$2<@CSNET-RELAY.ARPA>	user@host.CSNET

# handle other external cases
# R$+<@$->		$@$1<@$[$2$]>			no .ARPA on simple names
R$+<@$->		$@$1<@$2>			no .ARPA on simple names
R$+<@$+.$->		$@$1<@$2$3>			already ok (we think)
# R$+<@$+.$-.ARPA>	$@$1%$2<@$[$3.ARPA$]>		approximate something
R$+<@[$+]>		$@$1<@[$2]>			already ok

# convert remaining addresses to old format and externalize appropriately
R$+			$:$>5$1				=> old format
R$-:$+			$@$1.$2<@$A.$D>			convert berk hosts
R$*			$@$1<@$A.$D>			tack on our host name
