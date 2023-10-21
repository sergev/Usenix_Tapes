/*
**
**  Defs.h:  header file for rmail/smail.
**
**  Configuration options for rmail/smail.  As distributed, these are
**  your options:
** 	use gethostname() to determine hostname.
**	full domain name is 'hostname.uucp'.
**	path file is /usr/lib/uucp/paths.
**	no log, no record, no dbm, use sendmail.
** 
**  You can change these in the next few blocks.
**
*/

/*
**	@(#)defs.h	1.12   (UUCP-Project/CS)   6/29/86
*/

/*
**  If you select SENDMAIL below, you needn't worry about hostdomain here,
**  as rmail does no domain routing with a SNOOPY sendmail, and sendmail
**  itself sets hostdomain for smail.
**
**  Determining hostname:
**	To use gethostname(), define GETHOSTNAME.
**	To use uname(), define UNAME.
**	To use a literal string, define HOSTNAME "host"
**
**  Determining hostdomain:
**	To use hostname.mydomain, define MYDOM "mydomain"
**	To use a literal string, define HOSTDOMAIN "host.domain"
**	Otherwise, it will be "hostname" (not advised).
**
**  THE TOP LEVEL OF YOUR DOMAIN NAME IS IMPLICIT IN THE PATHALIAS DATABASE
*/

# define BSD				/* if system is a Berkeley system */
# define SENDMAIL 1			/* Turn off to use /bin/(l)mail only */

# ifdef BSD
# define GETHOSTNAME			/* use gethostname() */
# else
# define UNAME 				/* use uname() */
# endif
/* # define HOSTNAME   "host"		/* literal name */

/*
 * .UUCP here is just for testing, GET REGISTERED in COM, EDU, etc.
 * See INFO.REGISTRY for details.
 */
# define MYDOM	    ".UUCP"		/* literal domain suffix */

/* # define HOSTDOMAIN "host.dom"	/* replacement for HOSTNAME.MYDOM */

/*
**  Locations of files:
**	PATHS is where the pathalias output is.  This is mandatory.
**	Define LOG if you want a log of mail.  This can be handy for
**	debugging and traffic analysis.
**	Define RECORD for a copy of all mail.  This uses much time and
**	space and is only used for extreme debugging cases.
*/

#ifndef PATHS
# define PATHS	"/usr/lib/uucp/paths"	/* where the path database is */
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

				/* add -r for queueing */
# define RMAIL(from,sys)	"/usr/bin/uux - -r %s!rmail",sys
# define RARG(user)		" '(%s)'",user
# define RFROM(frm,now,host) 	"From %s  %.24s remote from %s\n",frm,now,host

#ifdef SENDMAIL
# define HANDLE	JUSTUUCP	/* see HANDLE definition below */
# define ROUTING JUSTDOMAIN	/* see ROUTING definition below */
/* # define ROUTING REROUTE	/* reroute everything (sets -R flag) */
# define LMAIL(frm,sys) 	"/usr/lib/sendmail -em -f%s",frm
# define LARG(user)		" '%s'",user
# define RLARG(sys,frm) " '%s!%s'",sys,frm
# define LFROM(frm,now,host) 	""
#else
# define HANDLE	ALL
# define ROUTING JUSTDOMAIN	/* */
/* # define ROUTING REROUTE	/* reroute everything (sets -R flag) */
#ifdef BSD
# define LMAIL(frm,sys)		"/bin/mail"	/* BSD local delivery agent */
#else
# define LMAIL(frm,sys)		"/bin/lmail"	/* SV  local delivery agent */
#endif
# define LARG(user)		" '%s'",user
# define RLARG(sys,frm) " '%s!%s'",sys,frm
# define LFROM(frm,now,host)	"From %s %.24s\n",frm,now
#endif SENDMAIL

/*
** DON'T TOUCH THE REST
*/

# define SMLBUF		512	/* small buffer (handle one item) */
# define BIGBUF		4096	/* handle lots of items */
 
# define MAXPATH	32	/* number of elements in ! path */
# define MAXDOMS	16	/* number of subdomains in . domain */
# define MAXARGS	32	/* number of arguments */

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
	DOMAIN, 	/* user@domain */
	UUCP,		/* host!address */
	ROUTE };	/* intermediate form - to be routed */

enum ehandle { 	/* what addresses can we handle? (don't kick to LMAIL) */
	ALL,		/* UUCP and DOMAIN addresses */
	JUSTUUCP,	/* UUCP only; set by -l  */
	NONE };		/* all mail is LOCAL; set by -L */

enum erouting {	/* when to route A!B!C!D */
	JUSTDOMAIN,	/* route A if A is a domain */
	ALWAYS,		/* route A always; set by -r */
	REROUTE };	/* route C, B, or A (whichever works); set by -R */

enum edebug {	/* debug modes */
	NO,		/* normal deliver */
	VERBOSE,	/* talk alot */
	YES };		/* talk and don't deliver */

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
