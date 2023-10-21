/*
 * show.c - TIG-pack picture show
 *
 * Usage:
 *      % show < spiral.dat | filter
 */

#include        <stdio.h>
#include        <math.h>

#define pi      3.1415926563
#define xcenter 3
#define ycenter 2.5


main()
	{
	extern double   getf();
	double          step , radius , angle , yliss , x , y ;
	int             i , n , ix , iy ;


    top:
	newform();
	grid();

    loop:
	step = getf();
	if ( step == 0.0 )
		goto top ;

	radius = getf();
	if ( radius == 0.0 )
		exit( 0 );

	radius = radius / 170.6667 ;

	yliss = getf();
	x = xcenter + radius ;
	ix = x * 1000 ;

	penup();
	iy = ycenter * 1000 ;
	movepen( ix, iy );
	pendown();

	for ( i = 1 ; i <= 128 ; ++ i )
		{
		angle = step * pi * i / 64.0 ;
		x = radius * cos( angle );
		y = radius * sin( yliss * angle );
		x = x + xcenter ;
		y = y + ycenter ;
		ix = x * 1000 ;
		iy = y * 1000 ;
		movepen( ix , iy );
		}
	goto loop ;
	}


static
double  getf()
	{
	char            buf[82];

	return( atof( fgets( buf , 82 , stdin ) ) ); 
	}


static
grid()
	{
	register int    i ;

	for ( i = 0 ; i <= 10 ; ++ i )
		{
		penup();
		movepen( 0 , i * 500 );
		pendown();
		movepen( 6000 , i * 500 );
		}
	for ( i = 0 ; i <= 12 ; ++ i )
		{
		penup();
		movepen( i * 500 , 0 );
		pendown();
		movepen( i * 500 , 5000 );
		}
	}
