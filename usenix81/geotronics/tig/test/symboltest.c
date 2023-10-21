/*
 * symboltest.c - just an exerciser for "symbol" and "marker"
 */

char    all[128];

main()
	{
	register int    i ;

	for ( i = 32 ; i < 128 ; ++ i )
		all[i-32] = i ;
	all[i-32] = 0 ;
	symbol( "Hi Doug!" , 1500 , 250 , 500 , 0.0 );
	symbol( "ABCDEFGHIJKLMNOPQRSTUVWXYZ" , 500 , 4000 , 150 , 0.0 );
	symbol( "abcdefghijklmnopqrstuvwxyz" , 1500 , 1500 , 150 , 0.0 );
	symbol( "*The Edge*" , 5600 , 500 , 400 , 90.0 );
	symbol( all , 250 , 750 , 50 , 45.0 );
	movepen( 3000 , 2000 );
	marker( 9 );
	movepen( 3000 , 2500 );
	marker( -9 );
	symbol( "Hi Doug!" , 4000 , 4750 , 500 , 180.0 );
	symbol( "\036" , 500 , 3000 , 1000 , 0.0 );
	symbol( "\036" , 500 , 3000 , 500 , 0.0 );
	symbol( "\036" , 500 , 3000 , 250 , 0.0 );
	symbol( "\036" , 500 , 3000 , 125 , 0.0 );
	}
