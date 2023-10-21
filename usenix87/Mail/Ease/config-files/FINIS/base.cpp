/***********************************************************
 ***********************************************************
 ****
 ****	General configuration information
 ****
 ****	This information is basically just "boiler-plate"; it must be
 ****	there, but is essentially constant.
 ****
 ****	Information in this file should be independent of location --
 ****	i.e., although there are some policy decisions made, they are
 ****	not specific to Berkeley per se.
 ****
 ****	$Id: base.cpp,v 1.3 85/05/04 17:00:43 acu Exp Locker: acu $
 ****
 ***********************************************************
 ***********************************************************/


#include "version.cpp"


/************************
 **   Special macros   **
 ************************/

macro
	m_daemon = "MAILER-DAEMON";
		/* my name */

	m_ufrom = "From ${m_sreladdr}  ${m_udate}";
		/* UNIX header format */

	m_addrops = ".:%@!^=/[]";
		/* delimiter (operator) characters */

	m_defaddr = concat ( 
			ifset (m_sname, "${m_sname} "),
			"<${m_sreladdr}>"
	    	    );
		/* format of a total name */

	m_smtp = "${m_oname} Sendmail ${m_version}/${berkhosts} ready at ${m_adate}";
		/* SMTP login message */


/***************
 **   Class   **
 ***************/

class
	uucphosts = {};

/*****************
 **   Options   **
 *****************/

options
	o_alias = "/usr/lib/aliases";
		/* location of alias file */

	o_ewait	= "10";
		/* wait up to ten minutes for alias file rebuild */

	o_bsub  = ".";
		/* substitution for space (blank) characters */

	o_delivery = d_background;
		/* default delivery mode (deliver in background) */

	/***
	o_qwait = "";
		/* (don't) connect to "expensive" mailers */

	o_tmode = "0600";
		/* temporary file mode */

	o_gid = "3";
		/* default GID (network) */

	o_fsmtp = "/usr/lib/sendmail.hf";
		/* location of help file */

	o_slog = "9";
		/* log level */

	/***
	o_dnet = "ARPA";
 		/* default network name */

	o_hformat = "";
 		/* default messages to old style */

	o_qdir = "/usr/spool/mqueue";
		/* queue directory */

	o_tread = "2h";
		/* read timeout -- violates protocols */

	o_flog = "/usr/lib/sendmail.st";
	 	/* status file */

	o_safe = "";
 		/* queue everything before transmission */

	o_qtimeout = "3d";
		/* default timeout interval */

	o_timezone = "EST";
 		/* time zone names (V6 only) */

	o_dmuid = "5";
 		/* default UID (network) */

	o_wizpass = "XXXXXXXXXXXXX";
	 	/* wizard's password */

	o_loadq = "999";
 		/* load average at which we just queue messages */

	o_loadnc = "999";
 		/* load average at which we refuse connections */


/*****************************
 **   Message precedences   **
 *****************************/

precedence
	first-class 	 = 0;
	special-delivery = 100;
	junk		 = -100;


/***********************
 **   Trusted users   **
 ***********************/

trusted
	{root, daemon, uucp, network};
	{aat};


/***************************
 **   Format of headers   **
 ***************************/

header
	define ("Received:", concat (ifset (m_shostname, "from ${m_shostname} "),
		"by ${m_oname}; ${m_adate}"));
	define ("Subject:", "");

	/*** 
	define ("Posted-Date:", "${m_odate}");
	 ***/

	for (f_return)
		define ("Return-Path:", "<${m_sreladdr}>");

	for (f_date) {
		define ("Resent-Date:", "${m_odate}");
		define ("Date:", 	"${m_odate}");
	};

	for (f_from) {
		define ("Resent-From:", "${m_defaddr}");
		define ("From:", 	"${m_defaddr}");
	};

	for (f_full)
		define ("Full-Name:", "${m_sname}");

	/***
	for (f_locm)
		define ("Received-Date:", "${m_adate}");
	 ***/

	for (f_mesg) {
		define ("Resent-Message-Id:", "<${m_ctime}.${m_qid}@${m_oname}>");
		define ("Message-Id:", 	      "<${m_ctime}.${m_qid}@${m_oname}>");
	};


/*************************
 *************************
 **   Rewriting rules   **
 ************************* 
 *************************/

/*************************
 **  Field definitions  **
 *************************/

field
	anypath			: match (0*);
	path, usr, hostpath,
	domain			: match (1*);
	this_host		: match (1) in m_sitename;
	hostname		: match (1);
	campushost		: match (1) in campushosts;
	localdomain		: match (1) in localname;
	topdomain_id		: match (1) in topdomain;
	uucphost		: match (1) in uucphosts;
	phonehost		: match (1) in phonehosts;

/********************************
 *  Sender Field Pre-rewriting  *
 ********************************/

ruleset SEND_PRW {

/***
	if ( anypath < anypath > anypath )
		retry ( $1$2$3 );			/* defocus */

}

/***********************************
 *  Recipient Field Pre-rewriting  *
 ***********************************/

ruleset RECP_PRW {

/***
	if ( anypath < anypath > anypath )
		retry ( $1$2$3 );			/* defocus */

}


/*********************************
 *  Final Output Post-rewriting  *
 *********************************/

ruleset FINAL_RW {

	if ( @ )
		return ();				/* handle <> error addr */

	/* externalize local domain info */

	/***
	if ( anypath < anypath "LOCAL" > anypath )
		retry ( $1 < $2 $localname > $3 );	/* change local info */
	
	/***
	if ( anypath < anypath "LOCAL.ARPA" > anypath )
		retry ( $1 < $2 $localname > $3 );	/* change local info */

	if ( anypath < path > anypath )
		retry ( $1$2$3 );			/* defocus */
		
	if ( @path: @path: usr )
		retry ( @$1,@$2:$3);			/* <route-addr> canonical */

	/* UUCP must always be presented in old form */

	if ( usr @ hostname ".UUCP" )
		retry ( $2!$1);				/* u@h.UUCP => h!u */

	/* delete duplicate local names -- mostly for arpaproto.mc */

	if ( usr % this_host @ this_host )
		retry ( $1@$3 );			/* u%UCB@UCB => u@UCB */

	if ( usr % this_host @ this_host ".ARPA" )
		retry ( $1@$3 ".ARPA" );		/* u%UCB@UCB => u@UCB */

}


/***************************
 *  Name Canonicalization  *
 ***************************/

ruleset NAME_CANON {

	/* handle "from:<>" special case */

	if ( <>	)
		return ( @ );				/* turn into magic token */

	/* basic textual canonicalization -- note RFC733 heuristic here */

	if ( anypath < anypath < anypath < path > anypath > anypath > anypath )
		retry ( $4 );				/* 3-level <> nesting */

	if ( anypath < anypath < path > anypath > anypath )
		retry ( $3 );				/* 2-level <> nesting */

	if ( anypath < path > anypath )
		retry ( $2 );				/* basic RFC821/822 parsing */

	if ( usr " at " path )
		retry ( $1@$2 );			/* "at" -> "@" for RFC 822 */

	/* make sure <@a,@b,@c:user@d> syntax is easy to parse -- undone later */

	if ( @path, usr )
		retry ( @$1:$2 );			/* change all "," to ":" */

	/* localize and dispose of route-based addresses */

	if ( @path: usr )
		return ( LOCAL_RW ( <@$1>:$2 ) );	/* handle <route-addr> */

	/* more miscellaneous cleanup */

	if ( path )
		next ( HOSTDEP_RW ( $1 ) );		/* host dependent cleanup */

	if ( path: anypath; @domain )
		return ( $1:$2;@$3 );			/* list syntax */

	if ( usr @ domain )
		next ( $1<@$2> );			/* focus on domain */

	if ( path < path @ domain > )
		retry ( $1$2<@$3> );			/* move gaze right */

	if ( path < @domain > )
		return ( LOCAL_RW ( $1<@$2> ) );	/* already canonical */

	/* convert old-style addresses to a domain-based address */

	if ( usr % hostpath )
		return ( LOCAL_RW ( $1<@$2> ) );	/* user%host */

	if ( hostname:usr )
		return ( LOCAL_RW ( $2<@$1> ) );	/* host:user */	

	if ( hostname.usr )
		return ( LOCAL_RW ( $2<@$1> ) );	/* host.user */

	if ( hostname^usr )
		retry ( $1!$2);				/* convert ^ to ! */

	if ( hostname!usr )
		return ( LOCAL_RW ( $2<@$1".UUCP"> ) );	   /* resolve uucp names */

	if ( hostname=usr )
		return ( LOCAL_RW ( $2<@$1".BITNET"> ) );  /* resolve bitnet names */
