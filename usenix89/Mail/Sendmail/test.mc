/*
 * Sendmail configuration file for test rulesets
 *
 * Simon Kenyon November 20th, 1986
 */

bind
	EnvelopeTo 		= ruleset  0;
	From			= ruleset  1;
	HeaderTo		= ruleset  2;
	Canonicalize		= ruleset  3;
	Externalize		= ruleset  4;

	LocalHeaderFrom		= ruleset 10;
	UucpHeaderFrom		= ruleset 11;

	LocalHeaderTo		= ruleset 20;
	UucpHeaderTo		= ruleset 21;

macro
	Domain		= "my_domain";
	Version		= "ruleset tester V1.0";

	m_sitename	= "whatever";
	m_oname		= "${m_sitename}.${Domain}";
	m_daemon	= "MAILER-DAEMON";
	/*m_ufrom		= "From ${m_sreladdr}  ${m_udate} remote from ${m_sitename}";*/
	m_ufrom		= "From ${m_sreladdr}  ${m_udate}";
	m_addrops	= ".:%@!^=/[]{}";
	m_defaddr	= concat (
				ifset (m_sname, "${m_sname}	<${m_sreladdr}>",
						"${m_sreladdr}"),
				""
			  );
	m_smtp		= "${m_oname} Sendmail ${m_version}/${Version} ready at ${m_adate}";

options
	o_alias		= "/usr/lib/aliases";
	o_delivery	= d_background;
	o_dmuid		= "1";
	o_flog		= "/usr/lib/sendmail.st";
	o_fsmtp		= "/usr/lib/sendmail.hf";
	o_gid		= "1";
	o_hformat	= "";
	o_qdir		= "/usr/spool/mqueue";
	o_qtimeout	= "3d";
	o_safe		= "";
	o_slog		= "9";
	o_timezone	= "WET";
	o_tmode		= "0644";
	o_tread		= "r2h";
	o_wizpass	= "*";

precedence
	first-class		=    0;
	special-delivery	=  100;
	junk			= -100;

trusted
	{root, daemon, uucp, network};
	{simon};

header
	define ("a:", "The origination date in Arpanet format = ${m_odate}");
	define ("b:", "The current date in Arpanet format = ${m_adate}");
	define ("c:", "The hop count = ${m_hops}");
	define ("d:", "The date in UNIX (ctime) format = ${m_udate}");
	define ("e:", "The SMTP entry message = ${m_smtp}");
	define ("f:", "The sender (from) address = ${m_saddr}");
	define ("g:", "The sender address relative to the recipient = ${m_sreladdr}");
	define ("h:", "The recipient host = ${m_rhost}");
	define ("i:", "The queue id = ${m_qid}");
	define ("j:", "The official domain name for this site = ${m_oname}");
	define ("l:", "The format of the UNIX from line = ${m_ufrom}");
	define ("n:", "The name of the daemon (for error messages) = ${m_daemon}");
	define ("o:", "The set of operators in addresses = ${m_addrops}");
	define ("p:", "Sendmail's pid = ${m_pid}");
	define ("q:", "The default format of sender address = ${m_defaddr}");
	define ("r:", "Protocol used = ${m_protocol}");
	define ("s:", "Sender's host name = ${m_shostname}");
	define ("t:", "A numeric representation of the current time = ${m_ctime}");
	define ("u:", "The recipient user = ${m_ruser}");
	define ("v:", "The version number of sendmail = ${m_version}");
	define ("w:", "The hostname of this site = ${m_sitename}");
	define ("x:", "The full name of the sender = ${m_sname}");
	define ("y:", "The id of the sender's tty = ${m_stty}");
	define ("z:", "The home directory of the recipient = ${m_rhdir}");

field
	path		: match (1*);

ruleset Canonicalize {
	if (path)
		next ("{3}" $1);
}

ruleset EnvelopeTo {
	if (path @ path)
		resolve (mailer (uucp),
			 host ("{0_uucp}" $2),
			 user ("{0_uucp}" $1));
	if (path ! path)
		resolve (mailer (uucp),
			 host ("{0_uucp}" $1),
			 user ("{0_uucp}" $2));
	if (path)
		resolve (mailer (local),
			 user ("{0_local}" $1));
}

ruleset From {
	if (path)
		return ("{1}" $1);
}

ruleset HeaderTo {
	if (path)
		return ("{2}" $1);
}

ruleset Externalize {
	if (path)
		return ("{4}" $1);
}

ruleset LocalHeaderFrom {
	if (path)
		return ("{S_local}" $1);
}

ruleset LocalHeaderTo {
	if (path)
		return ("{R_local}" $1);
}

ruleset UucpHeaderFrom {
	if (path)
		return ("{S_uucp}" $1);
}

ruleset UucpHeaderTo {
	if (path)
		return ("{R_uucp}" $1);
}

mailer
	local {
		Path		= "/usr/src/local/EUnet/ease/test/args",
		Flags		= {f_date,
				   f_from,
				   f_locm,
				   f_mesg,
				   f_mult,
				   f_noufrom,
				   f_rfrom,
				   f_strip},
		Sender		= LocalHeaderFrom,
		Recipient 	= LocalHeaderTo,
		Argv		= "args mail -d ${m_ruser}"
	};
	prog {
		Path		= "/usr/src/local/EUnet/ease/test/args",
		Flags		= {f_date,
				   f_expensive,
				   f_from,
				   f_locm,
				   f_mesg,
				   f_noufrom,
				   f_strip},
		Sender		= LocalHeaderFrom,
		Recipient 	= LocalHeaderTo,
		Argv		= "args sh -c ${m_ruser}"
	};
	uucp {
		Path		= "/usr/src/local/EUnet/ease/test/args",
		Flags		= {f_date,
				   f_from,
				   f_mesg,
				   f_strip,
				   f_ufrom,
				   f_upperh,
				   f_upperu},
		Sender		= UucpHeaderFrom,
		Recipient 	= UucpHeaderTo,
		Maxsize		= "65535",
		Argv		= "args uumail -h -oc -gA -f${m_sreladdr} ${m_rhost}!${m_ruser}"
	};
