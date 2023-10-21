/*
	opr -- off line output file dispatcher
		chooses spooling routine appropriate to destination

	last edit:	18-May-1981	D A Gwyn

1) added pen plotter support;
2) added graphic printer support.
*/


/* destinations - last entry in table is default */
char	*code[] =
	{
	"-gp" ,  "/lib/gps" ,	/* graphic printer */
	"-pp" ,  "/lib/pps" ,	/* pen plotter */
	"-lp" ,  "/lib/lpr" ,	/* line printer */
	0
	};

main( argc , argv )
	int	argc ;
	char	*argv[];
	{
	register int	i , j ;

	for ( i = 0 ; code[i] ; i += 2 )
		if ( argc > 1 )
			for ( j = 0 ; code[i][j] == argv[1][j] ; ++ j )
				if ( code[i][j] == 0 )
					execv( code[i+1] , &argv[1] );
	execv( code[i-1] , argv );
	write( 2 , "\07* opr * Can't start daemon\n" , 28 );
	}
