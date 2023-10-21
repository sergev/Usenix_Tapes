/*
**
**  Rmail/Smail - UUCP mailer with automatic routing.
**
**  Christopher Seiwald		/+\
**  cbosgd!chris		+\
**  January, 1985		\+/
**
*/

#ifndef lint
static char 	*sccsid="@(#)main.c	1.5  (UUCP-Project/CS)  6/9/86";
#endif

/*
**
**  usage:  	rmail [options] address...
**		smail [options] address...
**  options:
**		-d 	debug - verbose and don't invoke mailers.
**		-v	verbose - just verbose.
**		-h hostname	set hostname (default GETHOSTNAME,
**				UNAME, or HOSTNAME)
**		-H hostdomain	set hostdomain (default hostname.MYDOM)
**		-p pathfile	path database filename
**		(without both -r and -R, only user@domain gets routed)
**		-r	force routing of host!address
**		-R	reroute even explicit path!user
**		(without both -l and -L, only local mail goes local)
**		-l	user@domain goes to local mailer
**		-L	all mail goes local
**
**  -r, -R, -l, and -L can be preset in defs.h  Smail clears -l and -L.
**
*/

#include	<stdio.h>
#include	<ctype.h>
#include	"defs.h"

#ifdef UNAME
#include	<sys/utsname.h>
#endif


int exitstat = 0;		/* exit status, set by resolve, deliver	*/
enum edebug debug = NO;		/* set by -d or -v option		*/
enum ehandle handle = HANDLE;	/* which mail we can handle, see defs.h	*/
enum erouting routing = ROUTING;/* to route or not to route, see defs.h */
char hostname[SMLBUF] = "";	/* set by -h, defaults in defs.h 	*/
char hostdomain[SMLBUF] = "";	/* set by -H, defaults in defs.h 	*/
char *pathfile = PATHS;		/* or set by -p 			*/


/*
**
**  Rmail/Smail: mail stdin letter to argv addresses.
**
**  After processing command line options and finding our host and domain 
**  names, we map addresses into <host,user,form> triples.  Then we deliver.
**
*/

main( argc, argv )
int argc;
char **argv;
{
	char *hostv[MAXARGS];		/* UUCP neighbor 		*/
	char *userv[MAXARGS];		/* address given to host 	*/
	enum eform formv[MAXARGS];	/* invalid, local, or uucp 	*/
	char *p;
	int c;

	char *optstr = "dvrRlLH:h:p:";
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
**  Process command line arguments ( maybe getopt()? ).
*/
	while ((c = getopt(argc, argv, optstr)) != EOF) {
		switch ( c ) {
		case 'd': debug = YES; 			break;
		case 'v': debug = VERBOSE; 		break; 
		case 'r': routing = ALWAYS;		break;
		case 'R': routing = REROUTE;		break;
		case 'l': handle = JUSTUUCP;		break;
		case 'L': handle = NONE;		break;
		case 'H': (void) strcpy( hostdomain, optarg );	break;
		case 'h': (void) strcpy( hostname, optarg ); 	break;
		case 'p': pathfile = optarg; 		break;
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
**  Map argv addresses to <host, user, form>.
*/
	map( (argc - optind), &argv[optind], hostv, userv, formv );
/*
**  Deliver.
*/
	deliver( (argc - optind), hostv, userv, formv );
/*
**  Exitstat was set if any resolve or deliver failed, otherwise 0.
*/
	exit( exitstat );
}


/*
**
**  map(): map addresses into <host, user, form> triples.
**
**  Calls resolve() for each address of argv.  The result is hostv and 
**  userv arrays (pointing into buffers userz and hostz), and formv array.
**
*/

map( argc, argv, hostv, userv, formv )
int argc;				/* address count 		*/
char **argv;				/* address vector 		*/
char *hostv[];				/* remote host vector 		*/
char *userv[];				/* user name vector 		*/
enum eform formv[];			/* address format vector 	*/
{
	int i;
	enum eform resolve();
	char *c, *malloc();
	char *userz = malloc( BIGBUF );
	char *hostz = malloc( BIGBUF );

	for( i=0; i<argc; i++ )
	{
		userv[i] = userz;		/* put results here */
		hostv[i] = hostz;
		if ( **argv == '(' )		/* strip () */
		{
			++*argv;
			c = index( *argv, ')' );
			if (c)
				*c = '\0';
		}
						/* here it comes! */
		formv[i] = resolve( *argv++, hostz, userz );
		userz += strlen( userz ) + 1;	/* skip past \0 */
		hostz += strlen( hostz ) + 1;
	}
}


/*
**
**  getmynames(): what is my host name and host domain?
**
**  Hostname set by -h, failing that by #define HOSTNAME, failing
**  that by gethostname() or uname().
**  
**  Hostdomain set by -h, failing that by #define HOSTDOMAIN,
**  failing that as hostname.MYDOM, or as just hostname.
**
**  See defs.h for the inside story.
**
*/

getmynames()
{
#ifdef HOSTNAME
	if ( !*hostname )
		(void) strcpy( hostname, HOSTNAME );
#endif
#ifdef GETHOSTNAME
	if ( !*hostname )
		gethostname( hostname, sizeof( hostname ) - 1 );
#endif
#ifdef UNAME
	if ( !*hostname ) {
		struct utsname site;

		if ( uname( &site ) < 0 )
			error( EX_SOFTWARE, "uname() call failed", 0 );
		(void) strcpy( hostname, site.nodename );
	}
#endif
	if ( !*hostname )
		error( EX_SOFTWARE, "can't determine hostname.\n", 0 );
#ifdef HOSTDOMAIN
	if ( !*hostdomain )
		(void) strcpy( hostdomain, HOSTDOMAIN );
#endif
#ifdef MYDOM
	if ( !*hostdomain )
		(void) strcat( strcpy( hostdomain, hostname ), MYDOM );
#endif
	if ( !*hostdomain )
		(void) strcpy( hostdomain, hostname );

}


/*
**  ssplit(): split a line into array pointers.
**
**  Each pointer wordv[i] points to the first character after the i'th 
**  occurence of c in buf.  Note that each wordv[i] includes wordv[i+1].
**
*/

ssplit( buf, c, ptr )
register char *buf;		/* line to split up 		*/
char c;				/* character to split on	*/
char **ptr;			/* the resultant vector		*/
{
        int count = 0;
        int wasword = 0;

        for( ; *buf; buf++ )
        {
		if ( !wasword )
			count++, *ptr++ = buf;
		wasword = ( c != *buf );
        }
	if ( !wasword )
		count++, *ptr++ = buf;
        *ptr = NULL;
        return( count );
}
