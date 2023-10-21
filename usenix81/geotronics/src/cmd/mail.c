/*
	mail -- 6th Edition UNIX system mail-handling utility

	last edit: 05-Jan-1981	D A Gwyn

Usage:
	% mail[ -yn]			prints your mail
	% mail people			sends standard input to people
	% mail msgs			posts a message (UCB feature)

Installation:
	# cc -s -n -O mail.c -o /bin/mail
	# chmod 4711 /bin/mail
	# mkdir /usr/mail		: mailbox is /usr/mail/<name>
	# chmod 700 /usr/mail
*/

#include	<pwd.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<stdio.h>

extern struct passwd	*getpwnam(), *getpwuid();
extern char		*ctime(), *getlogin();
extern char		*strcat(), *strcpy(), *strncat();
extern int		isatty(), strcmp(), ttyslot();
extern int		access(), creat(), fork(), getgid(), getuid();
extern int		stat(), wait();
extern long		time();

#define	RWOWNER	0600			/* "creat" protection mode */
#define	WRITEAC	2			/* for "access" call */

static			onintr();
static int		accesss(), prepend();

static struct stat	inode ;		/* buffer for "stat" call */

static char	*lettmp = "/tmp/maXXXXXX" ;
static char	*preptmp = "/tmp/mbXXXXXX" ;

static int	errs = 0 ;		/* error resulting in loss */
static int	fromtty = 0 ;		/* input is from a terminal */
static int	saved = 0 ;		/* have made `dead.letter' */

static char	*msgs = "msgs" ;
static char	*prefix = "/usr/mail/" ;

#define	MAXNAME	20
static char	sender[MAXNAME+1];
static char	catbuf[MAXNAME+sizeof prefix];
main( argc , argv )			/* "mail" executive */
	int	argc ;
	char	*argv[];
	{
	register char		*myname ;
	register struct passwd	*p ;

	mktemp( lettmp );
	mktemp( preptmp );

	if ( (myname = getlogin()) == NULL )
		if ( (p = getpwuid( getuid() )) == NULL )
			{
			fprintf( stderr , "You don't exist.\n" );
			onintr( NSIG );
			}
		else
			myname = p->pw_name ;
	strncpy( sender , myname , MAXNAME );	/* save invoker name */

	++ argv ;
	if ( --argc <= 0 )
		goto printit ;
	for ( ; argc > 0 && (*argv)[0] == '-' ; --argc , ++argv )
		switch ( (*argv)[1] )
			{
		case 'y':
		case 'n':
    printit:
			printmail( argc , argv );
			onintr( 0 );
		/* ignore any other flags, for compatibility */
			}

	bulkmail( argc , argv );
	onintr( errs ? NSIG : 0 );
	}
static
printmail( argc , argv )		/* print existing mail */
	int	argc ;
	char	*argv[];
	{
	register int	c ;
	register char	*mname ;

	mname = strcat( strcpy( catbuf , prefix ) , sender );
	if ( stat( mname , &inode ) >= 0 && inode.st_nlink == 1 &&
	freopen( mname , "r" , stdin ) != NULL )
	/* no need to check for empty file; can't happen */
		{
		fcopy();

		c = 'y' ;
		if ( argc <= 0 )
			{
			if ( freopen( "/dev/tty" , "r" , stdin ) != NULL )
				{
				fprintf( stderr , "Save?" );
				c = getchar();
				}
			}
		else
			c = (*argv)[1];

		if ( c == 'y' )
			{
			if ( accesss( "mbox" ) )
				{
				prepend( mname , "mbox" ,
					 getuid() , getgid() );
				fprintf( stderr , "Saved mail in `mbox'\n" );
				unlink( mname );
				}
			}
		else if ( c == 'n' )
			unlink( mname );
		/* else keep original mail intact */
		}
	else
		fprintf( stderr , "No mail.\n" );
	}
static
bulkmail( argc , argv )		/* input to temp file; copy to people */
	int	argc ;
	char	*argv[];
	{
	long		tbuf ;

	unlink( lettmp );
	if ( signal( SIGHUP , SIG_IGN ) != SIG_IGN )
		signal( SIGHUP , onintr );
	if ( signal( SIGINT , SIG_IGN ) != SIG_IGN )
		signal( SIGINT , onintr );
	if ( signal( SIGQUIT , SIG_IGN ) != SIG_IGN )
		signal( SIGQUIT , onintr );
	if ( signal( SIGTERM , SIG_IGN ) != SIG_IGN )
		signal( SIGTERM , onintr );
	close( creat( lettmp , RWOWNER ) );	/* force mode */
	if ( freopen( lettmp , "w" , stdout ) == NULL )
		{
		perror( lettmp );
		onintr( NSIG );
		}

	fromtty = isatty( fileno( stdin ) );	/* input from where? */
	time( &tbuf );
	printf( "From %s  tty%d  %s\n" ,
		sender , ttyslot() , ctime( &tbuf ) );
	fcopy();
	putchar( '\n' );

	while ( argc-- > 0 )
		sendto( *argv++ );
	}
static
sendto( person )			/* copy temp file to person */
	char	*person ;
	{
	register struct passwd	*p ;
	int			status ;
	register int		i , j ;

	/* special "msgs" program feature: */
	if ( strcmp( person , msgs ) == 0 )
		{
		if ( (i = fork()) < 0 )
			{
			perror( "fork" );
			goto failed ;
			}
		if ( i == 0 )
			{		/* begin child process */
			freopen( "/dev/tty" , "w" , stdout );
			freopen( lettmp , "r" , stdin );
			execlp( msgs , msgs , "-s" , NULL );
			perror( msgs );
			exit( 100 );
			}		/* end child process */
		while ( (j = wait( &status )) != i )
			if ( j == -1 )
				goto failed ;
		if ( (status & 0377) != 0 || (status >> 8) == 100 )
			goto failed ;
		return ;
		}

	if ( (p = getpwnam( person )) != NULL &&
	     prepend( lettmp , strncat( strcpy( catbuf , prefix ) ,
					person , MAXNAME+sizeof prefix-1 ) ,
		      p->pw_uid , p->pw_gid )
	     != 0 )
		return ;

    failed:
	fprintf( stderr , "Can't send to %s\n" , person );
	++ errs ;
	/* Don't save more than once, nor if input was from file */
	if ( fromtty &&  saved++ == 0  && accesss( "dead.letter" ) )
		{
		prepend( lettmp , "dead.letter" , getuid() , getgid() );
		fprintf( stderr , "Letter saved in `dead.letter'\n" );
		}
	}
static
int prepend( from , to , ownuid , owngid )	/* prepend file to another */
	char	*from , *to ;
	int	ownuid , owngid ;
	{
	int		(*savehup)(), (*saveint)(),
			(*savequit)(), (*saveterm)();
	register int	ok = 0 ;

	unlink( preptmp );
	close( creat( preptmp , RWOWNER ) );	/* force mode */
	if ( freopen( preptmp , "w" , stdout ) == NULL )
		{
		perror( preptmp );
		onintr( NSIG );
		}
	if ( freopen( from , "r" , stdin ) == NULL )
		{
		perror( from );
		unlink( preptmp );
		return( 0 );
		}
	fcopy();
	freopen( to , "r" , stdin );	/* may not exist */
	fcopy();

	savehup = signal( SIGHUP , SIG_IGN );
	saveint = signal( SIGINT , SIG_IGN );
	savequit = signal( SIGQUIT , SIG_IGN );
	saveterm = signal( SIGTERM , SIG_IGN );
	unlink( to );
	close( creat( to , RWOWNER ) );	/* force mode */
	if ( freopen( to , "w" , stdout ) == NULL )
		goto restore ;
	chown( to , ownuid , owngid );
	if ( stat( to , &inode ) < 0 || inode.st_nlink != 1 )
		goto restore ;
	if ( freopen( preptmp , "r" , stdin ) == NULL )
		{
		perror( preptmp );
		++ errs ;
		goto restore ;
		}
	fcopy();
	++ ok ;

    restore:
	if ( ! ok )
		unlink( preptmp );
	signal( SIGHUP , savehup );
	signal( SIGINT , saveint );
	signal( SIGQUIT , savequit );
	signal( SIGTERM , saveterm );
	return( ok );
	}
// Utilities for "mail" program


static
onintr( sig )
	register int	sig ;
	{
	unlink( lettmp );
	unlink( preptmp );
	exit( sig );
	}


static
fcopy()					/* copy from stdin to stdout */
	{
	register int	c ;

	clearerr( stdout );
	while ( (c = getchar()) != EOF )
		{
		putchar( c );
		if ( ferror( stdout ) )
			{
			perror( "mail" );
			onintr( NSIG );
			}
		}
	}


static
int accesss( fnam )			/* test for writability */
	register char	*fnam ;
	{
	register int	ok ;

	ok = access( "." , WRITEAC ) != -1 &&
	     (stat( fnam , &inode ) < 0 || access( fnam , WRITEAC ) == 0);
	if ( ! ok )
		fprintf( stderr , "In wrong directory\n" );
	return ( ok );
	}
