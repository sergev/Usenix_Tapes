/*
 * line.c - TIG-pack library routine to plot a set of points
 *
 *	last edit: 03-Dec-1980	D A Gwyn
 *
 * This routine plots points connected by lines and/or as markers.
 * The "scale" routine is usually called before "line".
 *
 * Calling sequence:
 * line( x , y , npoints , mark , interval );
 *	where
 * unsigned x[],y[]	= x and y coordinates in 1/1000ths of an inch
 * int npoints		= number of points in x,y array
 * int mark		= character to be used as marker; can be any
 *			printing character, or 001 to 017 for special
 *			marker characters.  Additionally, the sign of
 *			`mark' determines the type of line to be drawn:
 *			-	Plot markers only
 *			0	Draw connecting lines only
 *			+	Draw lines and markers
 * int interval		= number of points between markers.
 *
 * Compile:
 *	# cc -c -O line.c
 *	# ar r /lib/libg.a line.o
 *
 * Usage:
 *	% cc application_program.c -lg
 */


line( x , y , npoints , mark , interval )
	unsigned	x[] , y[] ;
	int		npoints ;
	int		mark ;
	int		interval ;
	{
	int		lines , blobs ;	/* mode flags */
	register int	i ;		/* index x,y array */
	register int	counter ;	/* interval counter */

	if ( interval <= 0 )
		interval = 1 ;
	lines = mark >= 0 ;
	blobs = mark != 0 ;
	if ( mark < 0 )
		mark = - mark ;

	penup() ;
	for ( counter = i = 0 ; i < npoints ; ++ i )
		{
		movepen( x[i] , y[i] );
		if ( blobs )
			{
			if ( counter++ == 0 )
				marker( mark );
			counter %= interval ;
			}
		if ( lines )
			pendown();
		}
	penup();
	}
