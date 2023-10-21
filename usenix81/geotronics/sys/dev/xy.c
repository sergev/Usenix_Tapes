#
/*
 * xy.c - XY11 incremental plotter driver (H.I. DP-3 COMPLOT)
 *
 *	created 16-Jul-1980 by D A Gwyn
 */

#include "../h/param.h"
#include "../h/conf.h"
#include "../h/user.h"

#define	XYADDR	0172550			/* 0177530, 0172554 standard */
#define	BACKOFF	8			/* incrs to move away from limit sw */
#define	YLIMIT	(4400-4*BACKOFF)	/* max Y plot surface (0 is min) */
#define	SHEET	1700			/* sheet size in incrs */

#define	XYPRI	40
#define	XYLWAT	50
#define	XYHWAT	100

/* CSR bits: */
#define	DONE	0200			/* ready for command */
#define	IENABLE	0100			/* enable interrupts */

/* DBR bits: */
#define	PENUP	0040
#define	PENDN	0020
#define	NEGY	0010
#define	POSY	0004
#define	NEGX	0002
#define	POSX	0001

#define	FORM	0200			/* pseudo data */
#define	CLEAR	0201

/* mode flag bits: */
#define	ASLEEP	01000
#define	OPEN	00400
#define	RESET	00200

struct	{
	int	xycsr;
	int	xydbr;
	};

struct	{
	int	cc;
	char	*cf;
	char	*cl;
	int	flag;
	int	mx;			/* max abs x so far - init 0 */
	int	cx, cy;			/* current abs loc, unclipped */
	}
xy11;

/*
 * xyopen - open the device.  Write-only, single-user.
 */

xyopen( dev, flag )
	int	dev, flag;
	{
	if ( dev.d_minor || flag == 0 )
		{
		u.u_error = ENXIO;
		return;
		}
	if ( xy11.flag & OPEN )
		{
		u.u_error = EIO;
		return;
		}
	xy11.flag =| OPEN;
	XYADDR->xycsr =| IENABLE;
	xycanon( CLEAR );
	}

/*
 * xyclose - close the device.
 */

xyclose( dev, flag )
	int	dev, flag;
	{
	xycanon( FORM );
	xy11.flag =& ~ OPEN;
	}

/*
 * xywrite - write on device.
 */

xywrite( dev )
	int	dev;
	{
	register int	c;

	while ( (c = cpass()) >= 0 )
		xycanon( c );
	}

/*
 * xycanon - interpret character and perform action:
 *		CLEAR	does a FORM and forces pen to edge of paper
 *		FORM	advances to paper seam and leaves pen at (0,0)
 *		other	controls +_ X,Y,Z (pen first, then X,Y)
 */

xycanon( ch )
	int	ch;
	{
	register int	c, x, y;

	switch ( c = ch )
		{
	case CLEAR:
		if ( xy11.flag & RESET )
			return;		/* already reset */

		xy11.cy = YLIMIT;	/* force jam against -Y limit sw */
		xycanon( FORM );

		for ( y = 2 * BACKOFF; y; y-- )
			xyoutput( NEGY );
		while ( y++ < BACKOFF )
			xyoutput( POSY );

		xy11.flag =| RESET;
		return;

	case FORM:
		xyoutput( PENUP );	/* unconditional pen lift */
		xy11.flag =& ~ PENDN;
		xy11.flag =| PENUP;

		if ( x = xy11.mx % SHEET )
			xy11.mx =+ SHEET - x;	/* edge of next sheet */

		y = xy11.cy < YLIMIT ? xy11.cy : YLIMIT;
		for ( x = xy11.cx > 0 ? xy11.cx : 0; x < xy11.mx; x++ )
			{
			if ( y > 0 )
				{
				y--;
				c = POSX | NEGY;
				}
			else
				c = POSX;
			xyoutput( c );
			}

		for ( ; y > 0 ; y-- )
			xyoutput( NEGY );

		xy11.cx = xy11.cy = xy11.mx = 0;
		break;

	default:
		switch ( y = (x = c) & (PENUP | PENDN) )
			{
		case PENUP:
		case PENDN:
			if ( ! (xy11.flag & y) )
				{	/* pen in wrong state */
				xy11.flag =& ~ (PENUP | PENDN);
				xy11.flag =| y;	/* record new state */
				xyoutput( y );
				}
			}

		c = 0;			/* accumulates increment bits */

		switch ( y = x & (POSY | NEGY) )
			{
		case POSY:
			if ( ++xy11.cy > 0 && xy11.cy <= YLIMIT )
				c = y;
			break;
		case NEGY:
			if ( --xy11.cy >= 0 && xy11.cy < YLIMIT )
				c = y;
			}

		switch ( x =& POSX | NEGX )
			{
		case POSX:
			if ( ++xy11.cx > 0 )
				c =| x;
			if ( xy11.cx > xy11.mx )
				xy11.mx = xy11.cx;
			break;
		case NEGX:
			if ( --xy11.cx >= 0 )
				c =| x;
			}

		if ( c )
			xyoutput( c );
		}

	xy11.flag =& ~ RESET;
	}

/*
 * xyoutput - put character in output queue if room, otherwise wait.
 */

xyoutput( ch )
	int	ch;
	{
	spl4();
		while ( xy11.cc > XYHWAT )
			{
			xy11.flag =| ASLEEP;
			sleep( &xy11, XYPRI );
			}
		putc( ch, &xy11 );
		xystart();
	spl0();
	}

/*
 * xystart - put character(s) to plotter interface if ready.
 */

xystart()
	{
	register int	c;

	while ( XYADDR->xycsr & DONE && (c = getc( &xy11 )) >= 0 )
		XYADDR->xydbr = c;
	}

/*
 * xyintr - interrupt response code.  Executes at device priority.
 */

xyintr()
	{
	xystart();

	if ( xy11.cc <= XYLWAT && xy11.flag & ASLEEP )
		{
		xy11.flag =& ~ ASLEEP;
		wakeup( &xy11 );
		}
	}
