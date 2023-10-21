/*
 * scale.c - TIG-pack library routine to scale a set of data for plotting
 *
 *	last edit: 07-Dec-1980	D A Gwyn
 *
 * This routine scales an array of data (int, long, float, or double) to
 * fit in a designated distance when plotted.  Scaled data is ready for
 * the "line" routine.
 *
 * Calling sequence:
 * scale( &idata , npts , mode , length , &odata , &min , &delta );
 *	where
 * type idata[] 	= array of data to be scaled; `type' is
 *			  determined by `mode' parameter.
 * int npts		= number of elements in `idata' array.
 * char mode		= type of *idata:
 *	`i' - int	`l' - long	`f' - float	`d' - double
 * unsigned length	= minimum length of plotted array (in 1/1000s
 *			  of an inch).
 * int odata[]		= output array (always type int).
 * double *min		= minimum value of `idata'.
 * double *delta	= data value per 1/1000th inch increment.
 *			  idata / delta = odata.
 *			  "axis" routine input is delta*1000.0 .
 */

#include	<math.h>


scale( idata , npts , mode , length , odata , min , delta )
	int		*idata ;	/* or other type * */
	int		npts ;
	register char	mode ;
	unsigned	length ;
	int		*odata;
	double		*min ;
	double		*delta ;
	{
	double		x , xmax , xmin , dx ;
	register int	i ;		/* indexes arrays */
	long		*longp ;	/* aliases for idata */
	float		*floatp ;
	double		*doublep ;
	double		fract ; 	/* fractional part of delta */
	double		integ ; 	/* integral part of delta */

	/* set up several pointers of different types */
	doublep = floatp = longp = idata ;

	/* find min & max of array */
	xmin = HUGE ;
	xmax = -HUGE ;
	for ( i = 0 ; i < npts ; ++ i )
		{
		switch ( mode )
			{
		case 'i':
			x = (double)idata[i];
			break ;
		case 'l':
			x = (double)longp[i];
			break ;
		case 'f':
			x = (double)floatp[i];
			break ;
		case 'd':
		default:
			x = doublep[i];
			}
		if ( x > xmax )
			xmax = x ;
		if ( x < xmin )
			xmin = x ;
		}

	/* split raw delta into integral & fractional parts */
	fract = modf( log10( (xmax - xmin) / (double)length ) , &integ );
	if ( fract < 0.0 )
		{
		fract += 1.0 ;
		integ -= 1.0 ;
		}
	/* round to "nice" increment value */
	fract = pow( 10.0 , fract );
	switch( (int)(fract - 0.01) )
		{
	case 1:
		fract = 2.0 ;
		break ;
	case 2:
	case 3:
		fract = 4.0 ;
		break ;
	case 4:
		fract = 5.0 ;
		break ;
	case 5:
	case 6:
	case 7:
		fract = 8.0 ;
		break ;
	case 8:
	case 9:
		fract = 10.0 ;
		}

	/* compute plotting delta */
	dx = pow( 10.0 , integ ) * fract ;

	/* scale the data */
	for ( i = 0 ; i < npts ; ++ i )
		{
		switch ( mode )
			{
		case 'i':
			odata[i] = (int)((idata[i] - xmin) / dx + 0.5);
			break ;
		case 'l':
			odata[i] = (int)((longp[i] - xmin) / dx + 0.5);
			break ;
		case 'f':
			odata[i] = (int)((floatp[i] - xmin) / dx + 0.5);
			break ;
		case 'd':
		default:
			odata[i] = (int)((doublep[i] - xmin) / dx + 0.5);
			}
		}

	*min = xmin ;
	*delta = dx ;
	}
