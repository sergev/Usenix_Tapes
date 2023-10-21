/*
	spool -- simple-minded output spooler for spooling system


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

30-May-1981	D A Gwyn	Created.


Usage:
	Invoked directly or via "opr" to send files to output queue.
	Default is to link to file; "-c" flag copies files instead.
	"-r" flag removes files after queueing.  "-" is stdin file.

Protection:
	spooler:	mode 4711, set-UID root
	directory:	mode 755, owner daemon
	/tmp directory:	mode 777

Method:
	After checking read permission, each file is queued and the
	device daemon is awakened.  If link fails, copy is tried.
	Copy is done via a temporary file then "mv" to avoid races.
*/

#include	<signal.h>

#ifndef	DIR
#define	DIR	"/usr/lpd"		/* spool directory */
#define	DAEMON	"/etc/lpd"		/* device daemon */
#endif
#ifndef	UID
#define	UID	1			/* "daemon" uid */
#endif

extern char	*mktemp();
static		quit();

static char	*tmpfil , tmpnam[15];	// for file copy
static char	*lnkfil , lnknam[81];	// queue link name
static int	cflag = 0 ;		// -c option
static int	rflag = 0 ;		// -r option


main( argc , argv )
	int	argc ;
	char	*argv[];
	{
	signal( SIGHUP , SIG_IGN );
	if ( signal( SIGINT , SIG_IGN ) != SIG_IGN )
		signal( SIGINT , quit );
	if ( signal( SIGQUIT , SIG_IGN ) != SIG_IGN )
		signal( SIGQUIT , quit );
	if ( signal( SIGTERM , SIG_IGN ) != SIG_IGN )
		signal( SIGTERM , quit );

	//	Argument Processing

	while ( -- argc > 0 )
		if ( (*++argv)[0] != `-' )
			break ;	// filename
		else	{		// flag(s) or stdin "filename"
			register char	c ;
			register int	i = 0 ;

			if ( (*argv)[1] == 0 )
				break ;	// stdin "filename"

			while ( c = (*argv)[++i] )
				switch ( c )
					{
				case `c' :
					++ cflag ;
					break ;
				case `r' :
					++ rflag ;
					break ;
					}
			}
	if ( argc == 0 )
		{
		++ argc ;
		*argv = "-" ;		// stdin "filename" for cat
		}
#ifndef	DEBUG
	close( 2 );			// quiet errors
#endif

	//	File Spooling

	for ( ; -- argc >= 0 ; ++ argv )
		{
		register int	fd ;
		register int	pid , w ;
		int		status ;
		int		oops = 0 ;	// 1 => link failed
		int		si ;	// 1 => file is really stdin

		if ( si = strcmp( *argv , "-" ) == 0 )
			++ cflag ;	// can't link to stdin
		else
			if ( access( *argv , 4 ) )
				continue ;	// Oh, no you don't!

		strncpy( lnknam , DIR , 80 );
		strncat( lnknam , "/spoXXXXXX" , 80 );
		lnkfil = mktemp( lnknam );	// DIR/spoXXXXXX

		if ( ! cflag )		// try linking
			{
			if ( (pid = fork()) == 0 )
				{	// test daemon's access
				setuid( UID );
				setgid( 0 );
				_exit( access( lnkfil , 4 ) );
				}
			while ( (w = wait( &status )) != pid )
				if ( w == -1 )
					{
					status = w ;
					break ;
					}
			if ( status )
				{
				++ oops ;	// daemon can't read
				goto copy ;
				}

			if ( (pid = fork()) == 0 )
				{
				execl( "/bin/ln" ,
					"ln" , *argv , lnkfil , 0 );
				_exit( 127 );	// somebody stole ln!
				}
			while ( (w = wait( &status )) != pid )
				if ( w == -1 )
					{
					status = w ;
					break ;
					}
			if ( status )
				{
				unlink( lnkfil );
				++ oops ;	// try copy instead
				}
			}

    copy:	if ( cflag || oops )
			{
			strcpy( tmpnam , "/tmp/spoXXXXXX" );
			tmpfil = mktemp( tmpnam );
			if ( (fd = creat( tmpfil , 0600 )) < 0 )
				_exit( 1 );	// can't make temp file!
			close( fd );

			if ( (pid = fork()) == 0 )
				{
				close( 1 );
				if ( (fd = open( tmpfil , 1 )) != 1 )
					{
					if ( open( tmpfil , 1 ) != 1 )
						_exit( 2 );	// ??
					close( fd );
					}

				execl( "/bin/cat" , "cat" , *argv , 0 );
				_exit( 127 );	// somebody stole cat!
				}
			while ( (w = wait( &status )) != pid )
				if ( w == -1 )
					{
					unlink( tmpfil );
					_exit( 3 );	// "impossible"
					}
			if ( status )
				{
				unlink( tmpfil );
				_exit( 4 );	// "cat" failed!
				}

			if ( (pid = fork()) == 0 )
				{
				execl( "/bin/mv" ,
					"mv" , tmpfil , lnkfil , 0 );
				_exit( 127 );	// somebody stole mv!
				}
			while ( (w = wait( &status )) != pid )
				if ( w == -1 )
					{
					unlink( tmpfil );
					_exit( 5 );	// "impossible"
					}

			if ( status )
				{
				unlink( tmpfil );
				_exit( 6 );	// "mv" failed!
				}

			chown( lnkfil , UID , 0 );	// give to daemon
			}

		//	Remove original if requested

		if ( rflag && ! si )
			unlink ( *argv );

		//	Awaken daemon

		if ( (pid = fork()) == 0 )
			{
			execl( DAEMON , DAEMON , 0 );
			_exit( 127 );	// somebody stole daemon!
			}
		while ( (w = wait( &status )) != pid )
			if ( w == -1 )
				_exit( 7 );	// "impossible"
		if ( status )
			_exit( 8 );	// daemon failed!
		}

	_exit( 0 );			// success!
	}


static
quit()	{				// trap catcher
	if ( lnkfil )
		unlink( lnkfil );
	if ( tmpfil )
		unlink( tmpfil );

	_exit( 9 );
	}
