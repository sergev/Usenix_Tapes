/*
 * retab.c - detab/entab utility (can be used as filter).
 *
 *	last edit: 21-Dec-1980	D A Gwyn
 *
 * Trims lines and replaces white space with appropriate combination of
 * tabs and blanks.  Optional arguments are:
 *	-i		following arguments apply to input (detab).
 *	-o		following arguments apply to output (entab).
 *			[Both may be specified at once by -io .]
 *	m1 m2 ...	list of numeric tab stops (leftmost col is #1).
 *	+n		incremental tab stop spacing.
 *	filename	input/output file (default is standard i/o).
 * If no tab stops are specified, "9 +8" [DEC standard] is assumed.
 * If tab stops are given without being preceded by -i or -o, they
 * apply to both input and output.  Similarly, a filename not following
 * -i or -o is used for both input and output ("in-place" editing).
 */

#define	MAXLINE	256			/* largest tab stop */

#define	NO	0
#define	YES	1

#include	<signal.h>
#include	<stdio.h>

static char	*tmpnam	= "/tmp/rtXXXXXX";	/* temporary file */

static char	itab[MAXLINE], otab[MAXLINE];	/* i and o tab stops */

static	onintr(), error();


main( argc , argv )
	int		argc;
	char		*argv[];
	{
	char		line[MAXLINE], *s ;
	char		*iname , *oname ;	/* file names */
	int		iarg , oarg ;		/* doing i or o args */
	int		lastist , lastost ;	/* last i or o "m" stop */
	int		c , incr , tab ;
	register int	icol , ccol , ocol ;	/* column counters */

	for ( tab = 0 ; tab < MAXLINE ; ++ tab )
		itab[tab] = otab[tab] = NO ;	/* clear all tabs */

	lastist = lastost = 0 ;		/* no tab stops yet */
	iname = oname = NULL ;		/* no file names yet */
	iarg = oarg = NO ;		/* no -i or -o yet */

	while ( --argc > 0 )		/* process arguments */
		{
		if ( (*++argv)[0] == '-' )	/* -i or -o */
			{
			iarg = oarg = NO ;
			for ( s = &(*argv)[1] ; *s ; ++ s )
				switch ( *s )
					{
				    case 'i':
					iarg = YES ;
					break ;
				    case 'o':
					oarg = YES ;
					break ;
				    default:
					error( "illegal option" );
					}
			continue ;	/* next argument */
			}
		if ( iarg == NO && oarg == NO )
			iarg = oarg = YES ;	/* applies to both */

		if ( (*argv)[0] == '+' )	/* tab increment */
			{
			if ( (incr = atoi( &(*argv)[1] )) <= 0 )
				error( "bad increment" );
			if ( iarg )
				{
				if ( lastist >= MAXLINE )
					error( "too many increments" );
				for ( ; lastist < MAXLINE ;
						lastist += incr )
					itab[lastist] = YES ;
				}
			if ( oarg )
				{
				if ( lastost >= MAXLINE )
					error( "too many increments" );
				for ( ; lastost < MAXLINE ;
						lastost += incr )
					otab[lastost] = YES ;
				}
			}
		else if ( (*argv)[0] >= '0' && (*argv)[0] <= '9' )
			{
			if ( (tab = atoi( *argv ) - 1) <= 0 ||
					tab >= MAXLINE )
				error( "bad tab stop" );
			if ( iarg )
				itab[lastist = tab] = YES ;
			if ( oarg )
				otab[lastost = tab] = YES ;
			}
		else			/* assume it's a filename */
			{
			if ( iarg )
				if ( iname != NULL )
					error( "too many inputs" );
				else
					iname = *argv ;
			if ( oarg )
				if ( oname != NULL )
					error( "too many outputs" );
				else
					oname = *argv ;
			}
		}
	if ( lastist == 0 )
		for ( tab = 8 ; tab < MAXLINE ; tab += 8 )
			itab[tab] = YES ;	/* use defaults */
	if ( lastost == 0 )
		for ( tab = 8 ; tab < MAXLINE ; tab += 8 )
			otab[tab] = YES ;	/* use defaults */
	if ( iname != NULL )
		if ( freopen( iname , "r" , stdin ) == NULL )
			error( "can't open input file" );
	if ( oname != NULL )
		{
		mktemp( tmpnam );	/* invent file name */
		if ( signal( SIGINT , SIG_IGN ) != SIG_IGN )
			signal( SIGINT , onintr );
		if ( signal( SIGQUIT , SIG_IGN ) != SIG_IGN )
			signal( SIGQUIT , onintr );
		if ( signal( SIGTERM , SIG_IGN ) != SIG_IGN )
			signal( SIGTERM , onintr );
		signal( SIGHUP , SIG_IGN );
		if ( freopen( tmpnam , "w" , stdout ) == NULL )
			error( "can't create temp file" );
		}

	icol = ocol = 0 ;
	while ( (c = getchar()) != EOF )
		switch ( c )
			{
		    case '\b':
			if ( icol > 0 )
				-- icol ;
			break ;
		    case '\r':
		    case '\n':
		    case '\v':
		    case '\f':
			icol = ocol = 0 ;
			putchar( c );
			break ;
		    case ' ':
			++ icol ;
			break ;
		    case '\t':
			do
				++ icol ;
				while ( icol < MAXLINE && !itab[icol] );
			break ;
		    default:
			for ( ccol = ocol + 1 ; ccol <= icol ; ++ ccol )
				if ( ccol >= MAXLINE || otab[ccol] )
					{
					if ( ccol - ocol > 1 )
						putchar( '\t' );
					else
						putchar( ' ' );
					ocol = ccol ;
					}
			for ( ; ocol > icol ; -- ocol )
				putchar( '\b' );
			for ( ; ocol < icol ; ++ ocol )
				putchar( ' ' );
			if ( c >= ' ' )	/* assume DEL advances column */
				ocol = ++icol ;
			putchar( c );
			}

	if ( oname != NULL )
		{
		fclose( stdin );
		fclose( stdout );
		signal( SIGINT , SIG_IGN );	/* don't lose file! */
		signal( SIGQUIT , SIG_IGN );
		signal( SIGTERM , SIG_IGN );
		execl( "/bin/mv" , "mv" , tmpnam , oname , NULL );
		error( "somebody stole /bin/mv" );
		}
	exit( 0 );
	}


static
error( str )
	char		str[];
	{
	fprintf( stderr , "\07* retab * %s\n" , str );
	onintr();			/* clean up and quit */
	}


static
onintr()
	{
	unlink( tmpnam );		/* if any */
	exit( 1 );			/* return error status */
	}
