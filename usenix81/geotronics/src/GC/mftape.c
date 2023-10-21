/*
 * mftape - skip files on multiple-file tape,
 *	    copy next file to "tape.fil"
 *	This works on "vanilla" 6th edition UNIX
 *
 * Usage:
 *	% mftape files_to_be_skipped
 */

#define	MTDEV	"/dev/rmt0"
#define	MAXRECSIZE	10240	/* 20 blocks */
char	buf[MAXRECSIZE];

main( argc , argv )
	int		argc;
	char		*argv[];
	{
	int		file , i , o , skip ;
	register int	n ;

	if ( (i = open( MTDEV , 0 )) < 0 )
		{
		printf( "\07* mftape * can't open %s\n" , MTDEV );
		exit( 1 );
		}
	if ( (o = creat( "tape.fil" , 0640 )) < 0 )
		{
		printf( "\07* mftape * can't create tape.fil\n" );
		exit( 2 );
		}
	if ( argc <= 1 )
		skip = 0 ;
	else
		skip = atoi( *++argv );
	printf( "Skipping %d files...\n" , skip );
	for ( file = 0 ; file < skip ; file++ )
		{
		printf( "...file # %d:\n" , file );
		while ( (n = read( i , buf , MAXRECSIZE )) > 0 )
			;
		}
	printf( "Transfer file %d:\n" , file );
	while ( (n = read( i , buf , MAXRECSIZE )) > 0 )
		write( o , buf , n );
	close( i );
	close( o );
	exit( 0 );
	}
