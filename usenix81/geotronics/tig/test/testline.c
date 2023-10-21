/*
 * testline.c - test "line" library routine
 */
main()	{
	unsigned	x[5], y[5];
	int		i ;

	movepen( 1000 , 1000 );
	pendown();
	movepen( 1500 , 1000 );
	movepen( 1500 , 1500 );
	movepen( 1000 , 1500 );
	movepen( 1000 , 1000 );
	for ( i = 0 ; i < 5 ; ++ i )
		{
		x[i] = 1000 + 500 * i ;
		y[i] = 2000 + 100 * i ;
		}
	line( x , y , 5 , 4 , 2 );
	for ( i = 0 ; i < 5 ; ++ i )
		y[i] += 1000 ;
	line( x , y , 5 , 0 , 2 );
	for ( i = 0 ; i < 5 ; ++ i )
		y[i] += 1000 ;
	line( x , y , 5 , -4 , 2 );
	}
