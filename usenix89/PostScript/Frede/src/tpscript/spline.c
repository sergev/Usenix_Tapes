/*
 * spline.c
 * routine for converting troff's idea of splines into
 * the form that the postscript device can easily construct.
 * This means that the actual curves are not precisely those that would
 * be drawn on an other machine, but at least they fit the general
 * description and intention (see Pic manual, section 8).
 * The technique used here takes the starting point, the mid-points
 * of all intermediate lines, and the end point and uses these to construct
 * bezier spline fragments
 * If there are only two defining lines, i.e. two coordinate pairs, we have the
 * minimun requirements and use rcurveto from currentpoint to endpoint with
 * interpolation of the two midpoints of the lines as postcript's control
 * points.
 * With more than this we use several rcurveto instructions; the first and last
 * go from each end to the midpoint of the line second from each end, using
 * the midpoint and other end of the respective end lines as control points.
 * All other segments go from one midpoint to the next using the point where
 * the lines join for both control points
 *
 * It should be possible to get closer approximation by suitably chosing
 * control points at some other fraction of the way along the line,
 * rather than just taking mid- and endpoints. For example, the end
 * sections of multiple curves are not actually tangent at the mid-point
 * at the moment and are a bit lopsided.
 *
 */

#include	"tpscript.h"

#include	<ctype.h>

#define		MAXLINE		512		/* max len of D~ input line */
#define		MAXCOORD	50		/* max num x,y pairs */

		/*
		 * coordinates of each defining point in units, relative
		 * to the starting point
		 * the rounding errors are not cumulative so I don't think
		 * it needs to be floating point
		 */
typedef	struct	{
	int	x,
		y;
} COORD;

static	char	splineformat[] = "\n%d %d %d %d %d %d spln";

draw_spline( istr )
	FILE	*istr;
{
	register	int	nnums;
	register	char	*s;
	register	COORD	*cp;
	char	buf[MAXLINE];
	COORD	coord[MAXCOORD],
		current,
		next;

	fgets(buf, MAXLINE, istr);
	s = buf;
	cp = &coord[0];
		/*
		 * scan the input line for an indeterminate number of dx,dy
		 * pairs - some compilers cark if given sscanf with lots of
		 * args
		 */
	for( ; s < &buf[MAXLINE] && cp < &coord[MAXCOORD] ; cp++ )
	{
		while ( isspace( *s ) )
			s++;
		if ( *s == '\0' )
			break;		/* done */
		cp->x = atoi( s );
		while ( ! isspace( *s ) )
			s++;
		while ( isspace( *s ) )
			s++;
		if ( *s == '\0' )
			break;		/* done */
		cp->y = atoi( s );
		while ( ! isspace( *s ) )
			s++;
	}
	nnums = cp - &coord[0];
		/*
		 * now we go through and change all coordinate references to
		 * be relative to the starting point, rather than relative to
		 * each previous point - this assists conversion to postscript
		 * where each coordinate to an rcurveto is relative to the
		 * starting point (of each rcurveto)
		 * There is a bug in pic which only returns negatively
		 * one unit less than it went forward - this may lead to
		 * accumulating errors in a hideously complicated spline
		 * I dont want to *fix* pic in case that is only a fix(ughh)
		 * for a bug elsewhere
		 */
	for ( cp = &coord[1] ; cp < &coord[nnums] ; cp++ )
	{
		if ( cp->x < 0 )
			cp->x--;
		cp->x += cp[-1].x;
		if ( cp->y < 0 )
			cp->y--;
		cp->y += cp[-1].y;
	}

#ifdef	debug
	fprintf( stderr, "spline with %d coords to ", nnums );
	for ( cp = &coord[0] ; cp < &coord[nnums] ; cp++ )
		fprintf( stderr, "%d %d, ", cp->x, cp->y );
	putc( '\n', stderr);
#endif

	current.x = 0;
	current.y = 0;
		/*
		 * note: the starting point is always relative 0,0, and coord[0]
		 * is the other end of the first line
		 */
	cp = &coord[0];
	if ( nnums < 2 )
	{
		error( ERR_WARN, "Too few points in spline" );
		return;
	}
	if ( nnums == 2 )
	{
			/*
			 * remember: the positive y direction in troff is
			 * negative in postscript
			 */
		fprintf
		(
			postr, splineformat,
			cp->x/2, -(cp->y/2),
			(cp->x + coord[1].x)/2, -(cp->y + coord[1].y)/2,
			coord[1].x, -(coord[1].y)
		);
		return;
	}
		/*
		 * next current point will be middle of second line
		 */
	current.x = (cp->x + coord[1].x) / 2;
	current.y = (cp->y + coord[1].y) / 2;
	fprintf
	(
		postr, splineformat,
		cp->x/2, -(cp->y/2),
		cp->x, -(cp->y),
		current.x, -current.y
	);
	cp++;
	while( --nnums > 2 )
	{
			/*
			 * this defines the end of this curve section and the
			 * start of the following section ( relative to
			 * the starting point)
			 */
		next.x = (cp->x + cp[1].x) / 2;
		next.y = (cp->y + cp[1].y) / 2;
			/*
			 * note that each rcurveto must be done relative to
			 * the new currentpoint, thus the input relative
			 * values need to be adjusted
			 */
		fprintf
		(
			postr, splineformat,
			cp->x - current.x, current.y - cp->y,
			cp->x - current.x, current.y - cp->y,
			next.x - current.x, current.y - next.y
		);
		cp++;
		current = next;		/* set new current point */
	}

		/*
		 * finally we draw the last section
		 */
	fprintf
	(
		postr, splineformat,
		cp->x - current.x, current.y - cp->y,
		(cp->x + cp[1].x)/2 - current.x,
		current.y - (cp->y + cp[1].y)/2,
		cp[1].x - current.x, current.y - cp[1].y
	);
	return;
}

