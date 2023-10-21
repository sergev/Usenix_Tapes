/*
 * plottest.c - test "plot" TIG routine
 */

#include	<math.h>


main()
	{
	float	x[20], y[20];
	int	i ;

	for ( i = 0 ; i < 20 ; ++ i )
		{
		x[i] = (double)i ;
		y[i] = (x[i] == 0.0) ? 1.0 : sin( (double)x[i] ) / x[i];
		}

	plot( 1000 , 1000 , 4000 , 3000 , "x" , "sync(x)" , x , y , 20 );
	}
