/*
 * plot.c - TIG-pack library routine to make an X-Y plot
 *
 *	last edit: 07-Dec-1980	D A Gwyn
 *
 * This routine plots the given (x,y) points with specified parameters.
 *
 * Calling sequence:
 * plot( xo , yo , xlen , ylen , xtit , ytit , x , y , npts );
 *	where:
 * unsigned xo,yo	= location of origin (in 1/1000s of an inch)
 * unsigned xlen,ylen	= length of axes (in 1/1000s of an inch)
 * char xtit[],ytit[]	= axis labels
 * float x[npts],y[npts]= data to be plotted (float, not double)
 * unsigned npts	= number of data points
 *
 * Compile:
 *	# cc -c -O plot.c
 *	# ar r /lib/libg.a plot.o
 *
 * Usage:
 *	% cc application_program.c -lg -lm
 */

#include	<stdio.h>

struct coords
	{
	unsigned	x ;
	unsigned	y ;
	};


plot( org , len , xtit , ytit , xdata , ydata , npts )
	struct coords	org ;
	struct coords	len ;
	char		*xtit , *ytit ;
	float		xdata[], ydata[];
	int		npts ;
	{
	char		*calloc();
	int		*xsc , *ysc ;
	double		delx , dely , minx , miny ;

	if ( (xsc = calloc( npts , sizeof(*xsc) )) == NULL
	  || (ysc = calloc( npts , sizeof(*ysc) )) == NULL )
		{
		fprintf( stderr , "* plot alloc failure *\n" );
		exit( 1 );
		}

	scale( xdata , npts , 'f' , len.x , xsc , &minx , &delx );
	axis( xtit , org , len.x , 0.0 , 2 , minx , delx*1000.0 , 1000 );
	scale( ydata , npts , 'f' , len.y , ysc , &miny , &dely );
	axis( ytit , org , - len.y , 90.0 , 2 , miny , dely*1000.0 , 1000 );

	neworigin( org );
	line( xsc , ysc , npts , 3 , 1 );
	neworigin( 0 , 0 );

	free( xsc );
	free( ysc );
	}
