/*
 * testscale.c - test for "scale" TIG library routine
 */
main()	{
	unsigned	ux[101], uy[101];
	double		x[101], y[101], xmin, ymin, xdelta, ydelta ;
	int		i ;

	for ( i = -50 ; i <= 50 ; ++ i )
		{
		x[i+50] = ((double)i)*((double)i);
		y[i+50] = ((double)i)*x[i+50];
		}
	scale( x , 101 , 'd' , 5000 , ux , &xmin , &xdelta );
	scale( y , 101 , 'd' , 4000 , uy , &ymin , &ydelta );
	neworigin( 500 , 500 );
	line( ux , uy , 101 , 4 , 10 );
	}
