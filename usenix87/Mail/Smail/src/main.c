/*
**
**  rmail/smail - UUCP mailer with automatic routing.
**
**  Christopher Seiwald		/+\
**  chris@cbosgd.att.com	+\
**  January, 1985		\+/
**
*/

#ifndef lint
static char 	*sccsid="@(#)main.c	2.2 (smail) 1/26/87";
#endif

/*
**
**  usage:  	rmail [options] address...
**		smail [options] address...
**  options:
**		-d 		debug - verbose and don't invoke mailers.
**		-v		verbose - just verbose.
**		-h hostname	set hostname 
**		-H hostdomain	set hostdomain (default hostname.MYDOM)
**		-p pathfile	path database filename
**		-r		force routing of host!address
**		-R		reroute even explicit path!user
**		-l		user@domain goes to local mailer
**		-L		all mail goes local
**		-q number	mail queueing cost threshold
**		-m number	limit on number of uux_noqueue jobs
**		-u string	string of flags for uux
**		-a aliasfile	aliases filename (not used with SENDMAIL)
*/

#include	<stdio.h>
#include	<ctype.h>
#include	"defs.h"
#ifdef BSD
#include <strings.h>
#else
#include <string.h>
#endif

int exitstat = 0;		/* exit status, set by resolve, deliver	*/

enum edebug debug     = NO;	/* set by -d or -v option		*/
enum ehandle handle   = HANDLE;	/* which mail we can handle, see defs.h	*/
enum erouting routing = ROUTING;/* to route or not to route, see defs.h */

char hostname[SMLBUF]   = "";	/* set by -h, defaults in defs.h 	*/
char hostdomain[SMLBUF] = "";	/* set by -H, defaults in defs.h 	*/

char *pathfile  = PATHS;	/* or set by -p 			*/
char *uuxargs   = NULL;		/* or set by -u				*/
char *aliasfile = ALIAS;	/* or set by -a				*/
int  queuecost  = QUEUECOST;	/* or set by -q				*/
int  maxnoqueue = MAXNOQUEUE;	/* or set by -m                         */

char *spoolfile = NULL;		/* name of the file containing letter   */
FILE *spoolfp;			/* file pointer to spoolfile		*/
int  spoolmaster = 0;		/* indicates 'control' of spoolfile     */
void spool();


/*
**
**  rmail/smail: mail stdin letter to argv addresses.
**
**  After processing command line options and finding our host and domain 
**  names, we map addresses into <host,user,form,cost> sets.  Then we deliver.
**
*/

main( argc, argv )
int argc;
char *argv[];
{
	char *hostv[MAXARGS];		/* UUCP neighbor 		*/
	char *userv[MAXARGS];		/* address given to host 	*/
	int  costv[MAXARGS];		/* cost of resolved route	*/
	enum eform formv[MAXARGS];	/* invalid, local, or uucp 	*/
	char *p;
	int c;
	int nargc;
	char **nargv, **alias();

	char *optstr = "dvrRlLH:h:p:u:q:a:m:f:";
	extern char *optarg;
	extern int optind;

/*
**  see if we aren't invoked as rmail
*/
	if((p = rindex(argv[0], '/')) == NULL) {
		p = argv[0];
	} else {
		p++;
	}

	if(*p != 'r' ) {
		handle = ALL;
	}

/*
**  Process command line arguments
*/
	while ((c = getopt(argc, argv, optstr)) != EOF) {
		switch ( c ) {
		case 'd': debug      = YES; 		break;
		case 'v': debug      = VERBOSE; 	break; 
		case 'r': routing    = ALWAYS;		break;
		case 'R': routing    = REROUTE;		break;
		case 'l': handle     = JUSTUUCP;	break;
		case 'L': handle     = NONE;		break;
		case 'f': spoolfile  = optarg;		break;
		case 'p': pathfile   = optarg; 		break;
		case 'u': uuxargs    = optarg; 		break;
		case 'a': aliasfile  = optarg; 		break;
		case 'H': (void) strcpy(hostdomain, optarg);	break;
		case 'h': (void) strcpy(hostname, optarg); 	break;
		case 'm': if(isdigit(*optarg)) {
				  maxnoqueue = atoi(optarg);
			  }
			  break;
		case 'q': if(isdigit(*optarg)) {
				  queuecost = atoi(optarg);
			  }
			  break;
		default:
			error( EX_USAGE, "valid flags are %s\n", optstr);
		}
	}
	if ( argc <= optind ) {
		error( EX_USAGE, "usage: %s [flags] address...\n", argv[0] );
	}

/*
**  Get our default hostname and hostdomain.
*/
	getmynames();

/*
**  Spool the letter in a temporary file.
*/
	nargc = argc - optind;
	spool(nargc, &argv[optind]);

/*
** If we have sendmail, aliasing was done there, so alias() is a NOP.
** If we don't have sendmail, do our own aliasing.
*/
	nargv = alias(&nargc, &argv[optind]);

/*
**  Map argv addresses to <host, user, form, cost>.
*/
	map(nargc, nargv, hostv, userv, formv, costv);
/*
**  Deliver.
*/
	deliver(nargc, hostv, userv, formv, costv);
/*
**  Exitstat was set if any resolve or deliver failed, otherwise 0.
*/
	return( exitstat );
}
