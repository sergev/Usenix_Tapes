/*
 * axis.c - TIG-pack library routine to plot an axis for a graph
 *
 *	last edit: 07-Dec-1980	D A Gwyn
 *
 * This routine draws an axis with a linear scale and tic marks every
 * inch, labels the tics, and labels the axis with the supplied title.
 *
 * Calling sequence:
 * axis( string , x , y , length , theta , digits , minval , incv , unit );
 *	where:
 * char *string = string to be used as label for the axis.
 * int x,y	= coords (in 1/1000ths of an inch) of start of the axis.
 * int length	= length of the axis (1/1000ths).
 *		  length < 0 => tics on CCW side of axis.
 * double theta = CCW rotation of axis from the x-axis (in degrees).
 * int digits	= number of digits to be given after decimal point.
 * double minval= minimum value shown on the axis.
 * double incv	= the increment added to minval to label each additional
 *		  tic mark made on the axis.
 * int unit	= distance (in 1/1000ths) between tic marks.
 */

#include	<math.h>

#define PI		3.14159265358979323846264338327950288419716939937511
#define DEGRAD		(180.0/PI)	/* degrees per radian */

#define TIC_LEN 	80
#define NUM_HT		100
#define NUM_DIST	220
#define TIT_HT		120
#define TIT_DIST	400

/* rotation macros */
static double	cost , sint ;
#define X(x,y)	((x) * cost - (y) * sint)
#define Y(x,y)	((x) * sint + (y) * cost)
#define ROT(z)		{int temp;\
			temp = X( z.x - xy.x , z.y - xy.y ) + xy.x ;\
			z.y = Y( z.x - xy.x , z.y - xy.y ) + xy.y ;\
			z.x = temp ;}

struct	coords
	{
	int	x ;
	int	y ;
	};


axis( string , xy , length , theta , digits , minval , incv , unit )
	char		*string ;
	struct coords	xy ;
	int		length ;
	double		theta ;
	int		digits ;
	double		minval ;
	double		incv ;
	int		unit ;
	{
	register int	i ;		/* index  variable */
	struct coords	incr ;		/* increments for rotation */
	struct coords	bott ;		/* address of bottom of tic */
	struct coords	num ;		/* addr of tics number */
	struct coords	title ; 	/* addr of axis title */
#define CW	0
#define CCW	1
	register int	dir ;

	if ( length < 0 )
		{
		dir = CCW ;		/* tic direction */
		length = - length ;
		}
	else
		dir = CW ;

	cost = cos( theta / DEGRAD );
	sint = sin( theta / DEGRAD );

	bott.x = xy.x ;
	bott.y = xy.y + (dir == CW ? - TIC_LEN : TIC_LEN );
	ROT( bott )			/* bottom of tic */

	num.x = xy.x ;
	num.y = xy.y + (dir == CW ? - NUM_DIST : NUM_DIST );
	ROT( num )			/* tic label */

	title.x = xy.x + ( length - strlen( string ) * TIT_HT ) / 2 ;
	title.y = xy.y + (dir == CW ? - TIT_DIST : TIT_DIST);
	ROT( title )			/* axis label */

	incr.x = unit * cost ;		/* segment length */
	incr.y = unit * sint ;

	symbol( string , title , TIT_HT , theta );

	length = (length + unit - 1) / unit ;	/* number of tics to do */

	/* plot initial label and tic mark */
	number( minval , num , NUM_HT , theta , digits );
	movepen( bott );
	pendown();

	for ( i = 0 ; i < length ; ++ i )	/* rest of axis */
		{
		movepen( xy );
		pendown();

		xy.x += incr.x ;
		xy.y += incr.y ;
		bott.x += incr.x ;
		bott.y += incr.y ;
		num.x += incr.x ;
		num.y += incr.y ;
		minval += incv ;

		movepen( xy );		/* draw segment */

		movepen( bott );	/* draw tic */
		number( minval , num , NUM_HT , theta , digits );
		}
	}
