/*
 * ctest.c - complex arithmetic test
 *
 *	last edit: 10-Dec-1980	D A Gwyn
 */

#include	<stdio.h>
#include	<math.h>
#include	<complex.h>
#include	<std.h>


main()
	{
	complex	a , b , *cp ;

	/* CConst test */
	cp = CConst( 3.0 , 4.0 , &a );
	printf( "CConst; %f s.b. 3.0 , %f s.b. 4.0\n" , a.r , a.i );
	printf( "CConst; %f s.b. 3.0 , %f s.b. 4.0\n" , cp->r , cp->i );

	/* CCopy test */
	cp = CCopy( &a , &b );
	CConst( 1.0 , sqrt( 3.0 ) , &a );
	printf( "CCopy; %f s.b. 3.0 , %f s.b. 4.0\n" , b.r , b.i );
	printf( "CCopy; %f s.b. 3.0 , %f s.b. 4.0\n" , cp->r , cp->i );

	/* Modulus test */
	printf( "Modulus; %f s.b. 5.0\n" , Modulus( &b ) );

	/* Phase test */
	printf( "Phase; %f s.b. 60.0\n" , Phase( &a ) * DEGRAD );

	/* try other quadrants */
	a.r = - a.r ;
	printf( "Modulus; %f s.b. 2.0\n" , Modulus( &a ) );
	printf( "Phase; %f s.b. 120.0\n" , Phase( &a ) * DEGRAD );
	a.i = - a.i ;
	printf( "Modulus; %f s.b. 2.0\n" , Modulus( &a ) );
	printf( "Phase; %f s.b. -120.0\n" , Phase( &a ) * DEGRAD );
	a.r = - a.r ;
	printf( "Modulus; %f s.b. 2.0\n" , Modulus( &a ) );
	printf( "Phase; %f s.b. -60.0\n" , Phase( &a ) * DEGRAD );

	/* CPhasor test */
	cp = CPhasor( 100.0 , -20.0 / DEGRAD , &a );
	printf( "CPhasor; %f s.b. 100.0 , %f s.b. -20.0\n" ,
			Modulus( &a ) , Phase( &a ) * DEGRAD );
	printf( "CPhasor; %f s.b. 100.0 , %f s.b. -20.0\n" ,
			Modulus( cp ) , Phase( cp ) * DEGRAD );

	/* CConjug test */
	cp = CConjug( &b );
	printf( "CConjug; %f s.b. 3.0 , %f s.b. -4.0\n" , b.r , b.i );
	printf( "CConjug; %f s.b. 3.0 , %f s.b. -4.0\n" , cp->r , cp->i );

	/* CScale test */
	cp = CScale( 2.0 , &b );
	printf( "CScale; %f s.b. 6.0 , %f s.b. -8.0\n" , b.r , b.i );
	printf( "CScale; %f s.b. 6.0 , %f s.b. -8.0\n" , cp->r , cp->i );

	/* CAdd test */
	cp = CAdd( &b , CConst( -4.0 , 11.0 , &a ) );
	printf( "CAdd; %f s.b. 2.0 , %f s.b. 3.0 \n" , a.r , a.i );
	printf( "CAdd; %f s.b. 2.0 , %f s.b. 3.0 \n" , cp->r , cp->i );

	/* CSub test */
	cp = CSub( &b , CConst( 4.0 , 7.0 , &a ) );
	printf( "CSub; %f s.b. -2.0 , %f s.b. 15.0 \n" , a.r , a.i );
	printf( "CSub; %f s.b. -2.0 , %f s.b. 15.0 \n" , cp->r , cp->i );

	/* CMul test */
	cp = CMul( CConst( 1.0 , 2.0 , &a ) , CConst( -1.0 , 3.0 , &b ) );
	printf( "CMul; %f s.b. -7.0 , %f s.b. 1.0\n" , b.r , b.i );
	printf( "CMul; %f s.b. -7.0 , %f s.b. 1.0\n" , cp->r , cp->i );

	/* CDiv test */
	cp = CDiv( &a , &b );
	printf( "CDiv; %f s.b. -1.0 , %f s.b. 3.0\n" , b.r , b.i );
	printf( "CDiv; %f s.b. -1.0 , %f s.b. 3.0\n" , cp->r , cp->i );
	}
