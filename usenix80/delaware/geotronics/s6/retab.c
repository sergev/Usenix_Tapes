#
/*
 * retab.c -	detab/entab utility (can be used as filter).
 *
 * last edit: 10-Jun-1980  D A Gwyn
 *
 * Trims lines and replaces white space with appropriate combination of
 * tabs and blanks.  Optional arguments are:
 *	-i		following arguments apply to input (detab).
 *	-o		following arguments apply to output (entab).
 *	m1 m2 ...	list of numeric tab stops (col 1 is leftmost).
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

#define	PRMODE	0640			/* output file protection */
#define	RONLY	0			/* input file open mode */

char	*tmpnam	"retab.tmp";		/* temporary file */

char	itab[MAXLINE], otab[MAXLINE];	/* i and o tab stops */

main( argc, argv )
int	argc;
char	*argv[];
{
	char	*s;
	char	c, line[MAXLINE];
	char	*iname, *oname;		/* file names */
	char	iarg, oarg;		/* doing i or o args */
	int	lastist, lastost;	/* last i or o "m" stop */
	int	incr, tab;
	int	icol, ccol, ocol;	/* detab/entab col counters */

	for ( tab = 0;  tab < MAXLINE;  tab++ )
		itab[tab] = otab[tab] = NO;	/* clear all tabs */

	lastist = lastost = 0;		/* no tab stops yet */
	iname = oname = 0;		/* no file names yet */
	iarg = oarg = NO;		/* no -i or -o yet */

	while ( --argc > 0 )  {		/* process arguments */
		if ( (*++argv)[0] == '-' )  {	/* -i or -o */
			iarg = oarg = NO;
			for ( s = *argv + 1;  *s;  s++ )
				switch ( *s )  {
				case 'i':
					iarg = YES;
					break;
				case 'o':
					oarg = YES;
					break;
				default:
					Error( "illegal option" );
				}
			continue;	/* next argument */
		}
		if ( ! (iarg || oarg) )
			iarg = oarg = YES;	/* applies to both */

		if ( (*argv)[0] == '+' )  {	/* tab increment */
			if ( (incr = atoi( *argv + 1 )) <= 0 )
				Error( "bad increment" );
			if ( iarg )
				for ( tab = lastist;  tab < MAXLINE;
							tab =+ incr )
					itab[tab] = YES;
			if ( oarg )
				for ( tab = lastost;  tab < MAXLINE;
							tab =+ incr )
					otab[tab] = YES;
		}
		else if ( (*argv)[0] >= '0'  &&  (*argv)[0] <= '9' )  {
			if ( (tab = atoi( *argv ) - 1) <= 0  ||
					tab >= MAXLINE )
				Error( "bad tab stop" );
			if ( iarg )
				itab[lastist = tab] = YES;
			if ( oarg )
				otab[lastost = tab] = YES;
		}
		else  {			/* assume it's a filename */
			if ( iarg )
				if ( iname )
					Error( "too many inputs" );
				else
					iname = *argv;
			if ( oarg )
				if ( oname )
					Error( "too many outputs" );
				else
					oname = *argv;
		}
	}
	if ( lastist == 0 )
		for ( tab = 8;  tab < MAXLINE;  tab =+ 8 )
			itab[tab] = YES;	/* use defaults */
	if ( lastost == 0 )
		for ( tab = 8;  tab < MAXLINE;  tab =+ 8 )
			otab[tab] = YES;	/* use defaults */
	if ( iname )  {
		close( 0 );
		if ( open( iname, RONLY ) != 0 )
			Error( "can't open input file" );
	}
	if ( oname )  {
		close( 1 );
		if ( creat( tmpnam, PRMODE ) != 1 )
			Error( "can't create temp file" );
	}

	icol = ocol = 0;
	while ( read( 0, &c, 1 ) == 1 )
		switch ( c )  {
		case '\b':
			if ( icol > 0 )
				icol--;
			break;
		case '\r':
		case '\n':
		case 014:		/* form-feed */
			icol = ocol = 0;
			write( 1, &c, 1 );
			break;
		case ' ':
			icol++;
			break;
		case '\t':
			do
				icol++;
			while ( icol < MAXLINE  &&  ! itab[icol] );
			break;
		default:
			for ( ccol = ocol + 1;  ccol <= icol;  ccol++ )
				if ( ccol >= MAXLINE || otab[ccol] )  {
					if ( ccol - ocol > 1 )
						write( 1, "\t", 1 );
					else
						write( 1, " ", 1 );
					ocol = ccol;
				}
			for ( ;  ocol > icol;  ocol-- )
				write( 1, "\b", 1 );
			for ( ;  ocol < icol;  ocol++ )
				write( 1, " ", 1 );
			ocol = ++icol;
			write( 1, &c, 1 );
		}

	if ( oname )  {
		unlink( oname );
		if ( link( tmpnam, oname ) )
			Error( "can't rename temp file" );
		unlink( tmpnam );
	}
}


Error( str )
char	str[];
{
	int	len;

	write( 2, "* retab * ", 10 );
	for ( len = 0;  str[len];  len++ )
		;
	write( 2, str, len );
	write( 2, "\n", 1 );
	exit();
}
