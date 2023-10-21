




/*
 * erand - exponentially distributed random numbers with mean 'n'.
 */

erand(n)
int n ;
{
	double a,log() ;

	a = (urand() & 16383) ;
	a = -log( (a+1.0)/16384.0 ) * n ;
	n = (a + 0.5) ;

	return (n) ;

	} /* erand */
