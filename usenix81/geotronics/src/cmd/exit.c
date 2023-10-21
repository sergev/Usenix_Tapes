/*
	exit -- end shell script

Usage:
	% exit[ exit_status]
*/

main( argc , argv )
	register int	argc ;
	register char	*argv[];
	{
	lseek( 0 , 0L , 2 );
	if ( --argc > 0 )
		exit( atoi( ++argv ) );
	exit( 0 );
	}
