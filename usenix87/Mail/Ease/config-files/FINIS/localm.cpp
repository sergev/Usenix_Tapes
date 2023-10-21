/***********************************************************
 ***********************************************************
 ****
 ****	Local and Program Mailer specification
 ****
 ****	$Id: localm.cpp,v 1.4 85/07/10 22:38:09 acu Exp $
 ****
 ***********************************************************
 ***********************************************************/

mailer
	local {
		Path 	= "/bin/mail",
		Flags	= { f_rfrom, f_locm, f_strip, f_date, f_from,
			    f_mesg, f_mult, f_noufrom },
		Sender	= MAGIC_RW,
		Recipient = UNIMPLEMENTED,
		Argv	= "mail -d ${m_ruser}",
		Maxsize	= "200000"
	};

	prog {
		Path	= "/bin/sh",
		Flags	= { f_locm, f_strip, f_date, f_from, f_mesg, f_upperu },
		Sender	= MAGIC_RW,
		Recipient = UNIMPLEMENTED,
		Argv	= "sh -c ${m_ruser}",
		Maxsize	= "200000"
	};


ruleset MAGIC_RW {

	if ( @ )
		return ( "MAILER-DAEMON" );	/* errors to mailer-daemon */

}
