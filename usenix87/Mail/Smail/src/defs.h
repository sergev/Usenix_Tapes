/*
**
**  Defs.h:  header file for rmail/smail.
**
**  Configuration options for rmail/smail.
**	full domain name is 'hostname.uucp' (get registered!)
**	path file is /usr/lib/uucp/paths.
**	no log, no record, use sendmail.
** 
**  You can change these in the next few blocks.
**
*/

/*
**	@(#)defs.h	2.3 (smail) 1/26/87
*/

#ifndef VERSION
#define	VERSION	"smail2.3"
#endif

/*# define BSD				/* if system is a Berkeley system */

# define SENDMAIL "/usr/lib/sendmail"	/* Turn off to use /bin/(l)mail only */

/*
**  The ALIAS definitions are used only if SENDMAIL is NOT defined.
**  Sites using sendmail have to let sendmail do the aliasing.
**  ALIAS, must be defined, however, even if not used.
*/

# ifdef SENDMAIL
# define ALIAS	"not_used"
# else
# define ALIAS	"/usr/lib/aliases"	/* location of mail aliases    */
/*# define CASEALIAS			/* make aliases case sensitive */
# endif


# ifdef BSD
# define GETHOSTNAME			/* use gethostname() */
# else
# define UNAME 				/* use uname() */
# endif

/* if defined, HOSTNAME overrides UNAME and GETHOSTNAME */
/* # define HOSTNAME   "host"		/* literal name */

/*# define HOSTDOMAIN "host.dom"	/* replacement for HOSTNAME.MYDOM */

/*
 * .UUCP here is just for testing, GET REGISTERED in COM, EDU, etc.
 * See INFO.REGISTRY for details.
 */
# define MYDOM	    ".UUCP"		/* literal domain suffix */

/*
 * HIDDENHOSTS allows hosts that serve as domain gateways to hide
 * the subdomains beneath them.  Mail that originates at any of
 * the hosts in the subdomain will appear to come from the gateway host.
 * Hence, mail from
 *
 * 		anything.hostdomain!user
 *
 * will appear to come from 
 *
 * 		hostdomain!user
 *
 * A consequence is that return mail to hostdomain!user would need to
 * be forwarded to the proper subdomain via aliases or other forwarding
 * facilities.
 *
 * If you're using sendmail, then if defined here,
 * it should be used in ruleset 4 of the sendmail.cf, too.
 */

/*#define HIDDENHOSTS			/* hide subdomains of hostdomain */

/*
 * Mail that would otherwise be undeliverable will be passed to the
 * aliased SMARTHOST for potential delivery.
 *
 * Be sure that the host you specify in your pathalias input knows that you're
 * using it as a relay, or you might upset somebody when they find out some
 * other way.  If you're using 'foovax' as your relay, and below you have
 * #define SMARTHOST "smart-host", then the pathalias alias would be:
 *
 *	smart-host = foovax
 */

# define SMARTHOST  "smart-host"	/* pathalias alias for relay host */

/*
**  Locations of files:
**	PATHS is where the pathalias output is.  This is mandatory.
**	Define LOG if you want a log of mail.  This can be handy for
**	debugging and traffic analysis.
**	Define RECORD for a copy of all mail.  This uses much time and
**	space and is only used for extreme debugging cases.
*/

#ifndef PATHS
# define PATHS	"/usr/lib/uucp/paths"	/* location of the path database */
#endif

/*# define LOG 	"/usr/spool/uucp/mail.log"	/* log of uucp mail */
/*# define RECORD	"/tmp/mail.log"		/* record of uucp mail */

/*
**  Mailer options:
**	RMAIL is the command to invoke rmail on machine sys.
**	RARG is how to insulate metacharacters from RMAIL. 
**	LMAIL is the command to invoke the local mail transfer agent.
**	LARG is how to insulate metacharacters from LMAIL. 
**	RLARG is LARG with host! on the front - to pass a uux addr to sendmail.
**	SENDMAIL selects one of two sets of defines below for either
**	using sendmail or /bin/lmail.
*/	

#ifndef UUX
# define UUX			"/usr/bin/uux"	/* location of uux command   */
#endif

#ifndef SMAIL
# define SMAIL			"/bin/smail"	/* location of smail command */
#endif

/*
** command used to retry failed mail, flag is used to set the routing level.
*/
# define VFLAG		((debug == VERBOSE)?"-v":"")
# define RETRY(flag)	"%s %s %s -f %s ", SMAIL, VFLAG, flag, spoolfile

# define RMAIL(flags,from,sys)	"%s %s - %s!rmail",UUX,flags,sys
# define RARG(user)		" '(%s)'",user
# define RFROM(frm,now,host) 	"From %s  %.24s remote from %s\n",frm,now,host

#ifdef SENDMAIL

# define HANDLE	JUSTUUCP	/* see HANDLE definition below */
# define ROUTING JUSTDOMAIN	/* see ROUTING definition below */

# define LMAIL(frm,sys) 	"%s -em -f%s",SENDMAIL,frm
# define LARG(user)		" '%s'",postmaster(user)
# define RLARG(sys,frm)		" '%s!%s'",sys,frm
# define LFROM(frm,now,host)	"From %s %.24s\n",frm,now

#else

# define HANDLE	ALL
# define ROUTING JUSTDOMAIN

#ifdef BSD
# define LMAIL(frm,sys)		"/bin/mail"	/* BSD local delivery agent */
#else
# define LMAIL(frm,sys)		"/bin/lmail"	/* SV  local delivery agent */
#endif

# define LARG(user)		" '%s'",postmaster(user)
# define RLARG(sys,frm)		" '%s!%s'",sys,frm
# define LFROM(frm,now,host)	"From %s %.24s\n",frm,now

#endif

/*
**	The following definitions affect the queueing algorithm for uux.
**
**	DEFQUEUE	if defined the default is to queue uux mail
**
**	QUEUECOST	remote mail with a cost of less than QUEUECOST
**			will be handed to uux for immediate delivery.
**
**	MAXNOQUEUE	don't allow more than 'n' immediate delivery
**			jobs to be started on a single invocation of smail.
**	
*/

# define DEFQUEUE			/* default is to queue uux jobs */

# define QUEUECOST		100	/* deliver immediately if the cost
					/* is DEDICATED+LOW or better */

# define MAXNOQUEUE		2	/* max UUX_NOQUEUE jobs         */

# define UUX_QUEUE		"-r"	/* uux flag for queueing	*/
# define UUX_NOQUEUE		""	/* uux with immediate delivery	*/

/*
** Normally, all mail destined for the local host is delivered with a single
** call to the local mailer, and all remote mail is delivered with one call
** to the remote mailer for each remote host.  This kind of 'batching' saves
** on the cpu overhead.
**
** MAXCLEN is used to limit the length of commands that are exec'd by smail.
** This is done to keep other program's buffers from overflowing, or to
** allow for less intelligent commands which can take only one argument
** at a time (e.g., 4.1 /bin/mail).  To disable the batching, set MAXCLEN
** a small value (like 0).
*/

# define MAXCLEN		128	/* longest command allowed (approx.)
					/* this is to keep other's buffers
					** from overflowing
					*/

/*
** PLEASE DON'T TOUCH THE REST
*/

# define SMLBUF		512	/* small buffer (handle one item) */
# define BIGBUF		4096	/* handle lots of items */
 
# define MAXPATH	32	/* number of elements in ! path */
# define MAXDOMS	16	/* number of subdomains in . domain */
# define MAXARGS	500	/* number of arguments */
#ifndef NULL
# define NULL	0
#endif

# define DEBUG 			if (debug==YES) (void) printf
# define ADVISE 		if (debug!=NO) (void) printf
# define error(stat,msg,a)	{ (void) fprintf(stderr, msg, a); exit(stat); }
# define lower(c) 		( isupper(c) ? c-'A'+'a' : c )


enum eform {	/* format of addresses */
	ERROR, 		/* bad or invalidated format */
	LOCAL, 		/* just a local name */
	DOMAIN, 	/* user@domain or domain!user */
	UUCP,		/* host!address */
	ROUTE,		/* intermediate form - to be routed */
	SENT		/* sent to a mailer on a previous pass */
};

enum ehandle { 	/* what addresses can we handle? (don't kick to LMAIL) */
	ALL,		/* UUCP and DOMAIN addresses */
	JUSTUUCP,	/* UUCP only; set by -l  */
	NONE		/* all mail is LOCAL; set by -L */
};

enum erouting {	/* when to route A!B!C!D */
	JUSTDOMAIN,	/* route A if A is a domain */
	ALWAYS,		/* route A always; set by -r */
	REROUTE		/* route C, B, or A (whichever works); set by -R */
};

enum edebug {	/* debug modes */
	NO,		/* normal deliver */
	VERBOSE,	/* talk alot */
	YES		/* talk and don't deliver */
};

# ifdef BSD

# include	<sysexits.h>
# include	<strings.h>

# else

# include	"sysexits.h"
# include	<string.h>
# define	index	strchr
# define	rindex	strrchr

# endif
extern void exit(), perror();
extern unsigned sleep();
