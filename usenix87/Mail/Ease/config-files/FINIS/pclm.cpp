/****************************************************************************
 ****************************************************************************
 ****
 ****	PCL11 Mailer specification
 ****
 ****	Messages processed by this configuration are assumed to remain
 ****	in the same domain.  Hence, they may not necessarily correspond
 ****	to RFC822 in all details.
 ****
 ****	$Id: pclm.cpp,v 1.4 85/06/26 15:10:46 acu Exp $
 ****
 ****************************************************************************
 ****************************************************************************/

mailer
	pcl {
		Path	= "[IPC]",
		Flags	= { f_mult, f_date, f_from, f_mesg, f_upperu, 
			    f_addrw, f_dot, f_smtp },
		Sender	= HOSTNAME_RW,
		Recipient = HOSTNAME_RW,
		Argv	= "IPC ${m_rhost}",
		Maxsize = "100000"
	};

ruleset HOSTNAME_RW {

	if ( anypath < @path > anypath )
		return ( $1<@$2>$3 );			/* already ok */

	if ( path )
		return ( $1<@$m_sitename );		/* tack on our hostname */

}
