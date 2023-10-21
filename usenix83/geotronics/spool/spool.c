/*
	spool -- simple-minded output spooler for spooling system


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

30-May-1981	D A Gwyn	Created.
29-Apr-1982	D A Gwyn	Fixed some bugs, adapted to new "mv",
				put temp file in spool directory.
04-May-1982	D A Gwyn	Rearranged; added banner support.
05-May-1982	D A Gwyn	Fixed bug in spooling stdin.
06-May-1982	D A Gwyn	Report errors on stderr.
18-May-1982	D A Gwyn	Fixed interrupt catcher bug,
				avoid spooling empty files.


Usage:
	Invoked directly or via "opr" to send files to output queue.
	Usual name is "/lib/??s".

	Default is to link to file; "-c" flag copies files instead.
	"-r" flag removes files after queueing.  "-" file is stdin.

Protection:
	spooler:	mode 04711, set-UID root
	directory:	mode 00755, owner daemon
			(mode 00700 for extreme secrecy)

Method:
	After checking read permission, each file is queued and the
	device daemon is awakened.  If link fails, copy is tried.

	Define BANNER to be the name of the program that, given a login
	name argument, outputs a suitable banner on stdout (if wanted).
	7th Edition UNIX users need to invent their own version of the
	UNIX System III cuserid() routine if they want to have banners.

	Link/copy is done to a temporary file then "mv" to avoid races.
*/

#ifdef BANNER
#include	<stdio.h>		/* for cuserid() */
#else
#ifdef DEBUG
#include	<stdio.h>		/* for stderr */
#endif
#endif
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/stat.h>

#ifndef DIR
#define DIR	"/usr/lpd"		/* spool directory */
#endif

#ifndef DAEMON
#define DAEMON	"/etc/lpd"		/* device daemon */
#endif

#ifndef UID
#define UID	1			/* "daemon" uid */
#endif

#ifndef PFX
#define PFX	"spo"			/* data file prefix */
#endif

#ifndef TPFX
#define TPFX	"spt"			/* temporary file prefix */
#endif

#ifdef BANNER
#ifndef BPFX
#define BPFX	"spb"			/* banner file prefix */
/* NOTE: these three prefixes must have the same length */
#endif

extern char	*cuserid();
#endif
extern char	*mktemp(), *strcat(), *strcpy();
extern int	_exit(), access(), chown(), close(), creat(), execv(),
		fork(), open(), setgid(), setuid(), stat(), strlen(),
		unlink(), wait();

static		error(), onintr(), make();
static int	run();

typedef int	bool;
#define NO	0
#define YES	1

#ifndef NULL
#define NULL	(char *)0
#endif

static char	*prognam = NULL;	/* name of this program */
static char	*nulldev = "/dev/null";
static char	prefix[] = PFX; 	/* data file prefix */
static char	tmppfx[] = TPFX;	/* temporary file prefix */
#ifdef BANNER
static char	banpfx[] = BPFX;	/* banner file prefix */
#endif
#define DIRLN	sizeof DIR		/* (includes `/') */
#define PFXLN	(sizeof prefix - 1)
static char	filnam[DIRLN+PFXLN+7] = DIR;	/* more to follow */
static char	tmpnam[DIRLN+PFXLN+7] = DIR;	/* more to follow */
#ifdef BANNER
static char	bannam[DIRLN+PFXLN+7] = DIR;	/* more to follow */
static char	*banargs[] = { BANNER, NULL, NULL };
#endif
static char	*catargs[] = { "/bin/cat", NULL, NULL };
static char	*dargs[] = { DAEMON, NULL };
static char	*lnargs[] = { "/bin/ln", NULL, tmpnam, NULL };
static char	*mvargs[] = { "/bin/mv", tmpnam, filnam, NULL };
static bool	namesok = NO;		/* "filenames make sense" */
static bool	cflag = NO;		/* -c option */
static bool	rflag = NO;		/* -r option */

main( argc, argv )
	int	argc;
	char	*argv[];
	{
	signal( SIGHUP, SIG_IGN );

	prognam = *argv;		/* save for error reporting */

	if ( signal( SIGINT, SIG_IGN ) != SIG_IGN )
		signal( SIGINT, onintr );
	if ( signal( SIGQUIT, SIG_IGN ) != SIG_IGN )
		signal( SIGQUIT, onintr );
	if ( signal( SIGTERM, SIG_IGN ) != SIG_IGN )
		signal( SIGTERM, onintr );

	/* Argument Processing */

	while ( --argc > 0 )
		if ( (*++argv)[0] != '-' )
			break;		/* filename */
		else	{		/* flag(s) or stdin "filename" */
			register char	c;
			register int	i = 0;

			if ( (*argv)[1] == '\0' )
				break;	/* stdin "filename" */

			while ( (c = (*argv)[++i]) != '\0' )
				switch ( (int)c )
					{
				case 'c':
					cflag = YES;
					break;
				case 'r':
					rflag = YES;
					break;
					}
			}
	if ( argc == 0 )
		{
		++argc;
		*argv = "-";		/* fake filename for "cat" */
		}

	/* it would be nice if the following could be done statically */
	filnam[DIRLN-1] = tmpnam[DIRLN-1] =
#ifdef BANNER
			  bannam[DIRLN-1] =
#endif
					    '/';
	strcpy( &filnam[DIRLN], prefix );
	strcpy( &tmpnam[DIRLN], tmppfx );
#ifdef BANNER
	strcpy( &bannam[DIRLN], banpfx );

	if ( (banargs[1] = cuserid( NULL )) == NULL )
		banargs[1] = "????????";
#endif

	/* File Spooling */

	for ( ; --argc >= 0; ++argv )
		{
		register int	pid, w;
		int		status;
		bool		lfailed = NO;	/* "link failed" */
		bool		si = strcmp( *argv, "-" ) == 0;
					/* "file is really stdin" */

#ifdef DEBUG
		fprintf( stderr, "spool: spooling \"%s\":\n", *argv );
#endif
		if ( !si && access( *argv, 4 ) != 0 )
			continue;	/* someone else's file! */
		/* now we can operate safely with full privileges */

		strcpy( &filnam[DIRLN+PFXLN], "XXXXXX" );
		mktemp( filnam );	/* DIR/spo?????? */

		strcpy( &tmpnam[DIRLN+PFXLN], &filnam[DIRLN+PFXLN] );
					/* DIR/spt?????? */
#ifdef BANNER
		strcpy( &bannam[DIRLN+PFXLN], &filnam[DIRLN+PFXLN] );
					/* DIR/spb?????? */
#endif
		namesok = YES;
#ifdef DEBUG
		fprintf( stderr, " %s %s %s\n",
			 filnam, tmpnam, bannam
		       );
#endif

		if ( !si && !cflag )	/* link requested */
			{
#ifdef DEBUG
			fprintf( stderr, " link wanted\n" );
#endif
			if ( (pid = fork()) == 0 )
				{	/* test readability by daemon */
				setgid( 0 );
				setuid( UID );
				_exit( access( *argv, 4 ) );
				}
			while ( (w = wait( &status )) != pid )
				if ( w == -1 )
					{
					status = w;	/* -1 */
					break;
					}
			lfailed = status != 0;	/* if not readable */

			if ( !lfailed )
				{	/* attempt link */
#ifdef DEBUG
			fprintf( stderr, " attempt link\n" );
#endif
				lnargs[1] = *argv;
				if ( run( lnargs, YES, nulldev ) != 0 )
					{
					unlink( tmpnam );
					lfailed = YES;	/* try copy */
					}
				}
			}

		if ( si || cflag || lfailed )
			{
#ifdef DEBUG
			fprintf( stderr, " attempt copy\n" );
#endif
			make( tmpnam ); /* create temporary file */

			catargs[1] = *argv;
			if ( run( catargs, NO, tmpnam ) != 0 )
				error( catargs[0] );
			}

		/* if here, successfully made link or copy */

		{
		struct stat	statbuf;

		if ( stat( tmpnam, &statbuf ) != 0 )
			error( "stat" );
		if ( statbuf.st_size == 0L )	/* empty file */
			{
#ifdef DEBUG
			fprintf( stderr, " empty file\n" );
#endif
			unlink( tmpnam );
			if ( !si && rflag )	/* remove original */
				unlink( *argv );

			continue;	/* not spooled */
			}
		}

#ifdef BANNER
#ifdef DEBUG
		fprintf( stderr, " make banner file\n" );
#endif
		make( bannam ); 	/* create banner file */

		if ( run( banargs, YES, bannam ) != 0 )
			error( banargs[0] );
#endif

		if ( run( mvargs, YES, nulldev ) != 0 ) /* rename */
			error( mvargs[0] );

		if ( !si && rflag )	/* remove original file */
			unlink( *argv );

#ifdef DEBUG
		fprintf( stderr, " start daemon\n" );
#endif
		run( dargs, YES, nulldev );	/* awaken daemon */
		/* if daemon fails, keep going since files are safe */
		}

	_exit( 0 );			/* success! */
	}

static
make( name )				/* make a new file */
	char	*name;			/* name of file */
	{
	register int	fd = creat( name, 0600 );

#ifdef DEBUG
	fprintf( stderr, "spool: make \"%s\"\n", name );
#endif
	if ( fd < 0 )
		error( name );		/* can't make file! */
	close( fd );

	chown( name, UID, 0 );		/* now belongs to daemon */
	}

static int
run( args, closein, outname )		/* run a program */
	char	*args[];		/* arguments for execv() */
	bool	closein;		/* "close stdin" flag */
	char	*outname;		/* filename for stdout */
	{
	static int	status = -1;	/* child status from "wait" */
	register int	pid;		/* forked process ID */
	register int	w;		/* "wait" result */

#ifdef DEBUG
	fprintf( stderr, "spool: run %s >%s\n", args[0], outname );
#endif
	if ( (pid = fork()) == 0 )
		{
		setuid( 0 );		/* needed for new "mv" etc. */

		if ( closein )
			close( 0 );	/* protect against runaways */
		close( 1 );
		close( 2 );		/* detect exit status instead */
		if ( closein && open( nulldev, 0 ) != 0
		  || open( outname, 1 ) != 1
		  || execv( args[0], args ) != 0
		   )
			_exit( 127 );	/* execution cannot succeed */
		}

	while ( (w = wait( &status )) != pid )
		if ( w == -1 )
			error( "wait" );	/* "impossible" */

#ifdef DEBUG
	fprintf( stderr, "  status %d\n", status );
#endif
	return status;
	}

static
error( what )				/* error; clean up and exit */
	char	*what;			/* what failed */
	{
	signal( SIGINT, SIG_IGN );
	signal( SIGQUIT, SIG_IGN );
	signal( SIGTERM, SIG_IGN );

	if ( namesok )			/* remove any garbage */
		{
		unlink( filnam );
		unlink( tmpnam );
#ifdef BANNER
		unlink( bannam );
#endif
		}

	write( 2, prognam, strlen( prognam ) ); /* avoid stdio */
	write( 2, ": ", 2 );
	write( 2, what, strlen( what ) );
	write( 2, " failed\n", 8 );

	_exit( 1 );			/* report failure */
	}

static
onintr( sig )				/* called on signal */
	register int	sig;		/* caught signal number */
	{
	char	message[28];

	signal( sig, SIG_IGN ); 	/* just in case */

	strcpy( message, sig == SIGINT	? "SIGINT"  :
			 sig == SIGQUIT ? "SIGQUIT" :
			 sig == SIGTERM ? "SIGTERM" :
					  "signal"
	      );
	strcat( message, " received => spooler" );
	error( message );
	}
