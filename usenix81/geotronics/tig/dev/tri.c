/*
	tri -- TIG-pack device interpreter for Trilog Colorplot-100

	Based loosely on Mike Muuss's Versatec interpreter.

	last edit: 22-Jan-1981	D A Gwyn

Function:
	Reads TIG device-independent data from files given as arguments,
	if any, otherwise reads standard input.  Builds a scratch file
	containing Trilog plot codes for each frame and spools via lpr.
	Edge-limiting is done here; use "rot" if clipping is required.
	Color plotting is not yet supported.

Compile:
	# cc -s -n -O -DXCHUNK=25 -DOPTIMIZE tri.c -o /usr/bin/tri
	# chmod 755 /usr/bin/tri

Usage:
	% tri input_file_names
	or
	% tri < input
	or
	% main.out | tri

Method:
	Inputs vector data and builds a rasterization descriptor for
	each visible vector.  (Vectors are limited to page bounds.)
	X goes down the page, Y goes from left to right.

	The page image of NSCANS rasters stored on disk is considered
	to be divided into NBANDS bands, each containing XCHUNK
	rasters.  Each band has a linked list of descriptors for
	not-yet-rasterized vectors that begin in the band.

	Space for descriptors is obtained via "malloc".  When no more
	space is available, the disk image is updated as follows, then
	"malloc" is tried again ("must" work this time):

	Each band in increasing X order becomes "active"; if no
	descriptors exist for the band it is skipped, else its existing
	raster data is read from disk into a buffer and each descriptor
	is used to rasterize its vector.  If the vector terminates in
	the band its descriptor is freed, otherwise the descriptor is
	linked into the following band.  When the descriptor list for
	the active band becomes empty (must happen), the band's raster
	data is flushed back to the disk file and the next band becomes
	active.  This continues until all vectors have been rasterized.
*/
/* define the following via the C preprocessor */
//#define	DEBUG				// remove for production
//#define	OPTIMIZE			// run spool file optimizer
//#define	COLORPLOT			// not yet supported
//#define	OKIDATA				// requires full plot line
//#define	XCHUNK		25
//#define	XPI		100
//#define	YPI		100

/* include header files */
#include	<signal.h>
#include	<stdio.h>
#ifdef	COLORPLOT
#include	<TIGcolors.h>
#endif

/* device parameters */
// The Trilog has 100 nibs/inch in both directions
// and is 13.2 inches wide by 11 inches high.
#ifndef	XPI
#define	XPI		100		/* pixels per inch down page */
#define	YPI		100		/* pixels per inch across page*/
#endif
#define	NSCANS		(11*XPI)	/* = 2 * 2 * 5 * 5 * 11 (Trilog) */
#ifndef	XCHUNK
#define	XCHUNK		25		/* tweak for fine-tuning */
#endif
#define NBANDS		((NSCANS+XCHUNK-1)/XCHUNK)
#define	XMAX		(NSCANS-1)
#define	RASTERBITS	(132*YPI/10)	/* plot bits per scan (132 columns) */
#define	BITS		6		/* plot bits per byte */
#define BYTES		(RASTERBITS/BITS)	/* bytes per scan */
#define	YMAX		(RASTERBITS-1)

#define PLOTMODE	0005		/* plot-mode character */
#define	BIT6		0100		/* indicates plot code */

#ifdef	COLORPLOT
#define	SPOOLER	"/lib/cpr"
#else
#define	SPOOLER	"/lib/lpr"
#endif

typedef	char	tiny ;
typedef	int	bool ;
#define	NO		0
#define	YES		1
typedef struct
	{
	unsigned	x ;
	unsigned	y ;
	}	coords ;		// TIG "UPA" coordinates

typedef struct descr
	{
	struct descr	*nextv ;	// next in linked list, or NULL
	coords		nib ;		// starting X, Y pixel coords
#ifdef	COLORPLOT
	tiny		colors ;	// vector color bits
#endif
	tiny		xsign ;		// 0 or +1
	tiny		ysign ;		// -1 , 0 , or +1
	bool		xmajor ;	// TRUE iff X is major direction
	int		major ;		// major dir delta (nonneg)
	int		minor ;		// minor dir delta (nonneg)
	int		e ;		// DDA error accumulator
	int		de ;		// increment for `e'
	}	vect ;			// rasterization descriptor

/*	global data	*/

// When implementing color plotting, bit-encode Cyan, Magenta, & Yellow
// in `colors' in each vector descriptor.  Change the disk file to have
// three sections separated by (reverse-FF,color-select) codes.  As each
// color is updated during "outbuild", clear the corresponding bit in
// `colors' and free the descriptor when all bits are 0.  A separate
// spooler for color-ribbon printing (or a print queue manager) will be
// required.
#ifdef	COLORPLOT
static short	color = BLACK ;		// current color
#endif

static vect	*band[NBANDS];		// descriptor lists for each band

// active band buffer:
static char	buffer[XCHUNK][BYTES+2];	// allow for 05 , 012
static unsigned	xstart ;		// active band starting X nib

static char	iname[] = "/usr/tmp/triXXXXXX";	// for "mktemp"
static char	oname[] = "/usr/tmp/troXXXXXX";
static FILE	*ifp ;			// for temp files
static FILE	*ofp ;

extern char	*malloc();

static		onintr();
static vect	*allocate();
static vect	*dequeue();
/*
	main - "tri" executive

	This routine reads TIG commands from the appropriate input and
	controls the entry of the vectors into the descriptor lists.
	Vectors are limited (not clipped) to page bounds.
*/


main( argc , argv )
	int	argc ;
	char	*argv[];
	{
	bool	drawn ;			// NO => point needed
	bool	plotted ;		// NO => empty page image
	int	c ;			// input character
	int	pid ;			// process ID from fork
	int	status ;		// for "wait" system call
	int	virpen ;		// virtual pen state
	coords	virpos ;		// virtual pen position
	coords	newpos ;		// current input coordinates

	mktemp( iname );		// invent unique names
	mktemp( oname );

	if ( argc < 2 && isatty( fileno( stdin ) ) )
		bomb( "missing input" , NULL );
	argv[0] = "stdin";		// for "bomb" message

	signal( SIGHUP , SIG_IGN );
	if ( signal( SIGINT , SIG_IGN ) != SIG_IGN )
		signal( SIGINT , onintr );
	if ( signal( SIGQUIT , SIG_IGN ) != SIG_IGN )
		signal( SIGQUIT , onintr );
	signal( SIGTERM , onintr );

	do	{
		/* if no file specs, use standard input */
		if ( --argc > 0 && freopen( *++argv , "r" , stdin ) == NULL )
			{
			fprintf( stderr , "\07* tri * \"%s\" not found\n" ,
			*argv );
			goto nextfile ;
			}
    newframe:	initfile();		// create empty page image
		{
		register vect	**bp;	// *bp -> start of band descr list

		for ( bp = &band[0] ; bp < &band[NBANDS] ; ++ bp )
			*bp = NULL ;	// nothing in band yet
		}
		virpos.x = virpos.y = 0 ;
		virpen = `U' ;
#ifdef	COLORPLOT
		color = BLACK ;
#endif
		drawn = YES ;
		plotted = NO ;

		for ( ; ; )
			switch ( c = getchar() )
				{
			case EOF:
			case `F':	/* new frame */
				if ( ! drawn )
					{
					buildvec( &virpos , &virpos );
					plotted = drawn = YES ;
					}

				if ( plotted )
					{
					outbuild();	// flush vects
#ifdef	OPTIMIZE
					optimize();	// shrink file
#endif
					if ( (pid = fork()) == 0 )
						{
						// spool by linking
						chmod( oname , 0644 );
						execl( SPOOLER ,
						"lpr" , "-r" , oname , 0 );
						bomb( "somebody stole %s" ,
						SPOOLER );
						}
					if ( pid == -1 )
						bomb( "can't spool %s" ,
						*argv );
					wait( &status );
//					if ( status != 0 )
//						bomb( "spooler failed: %s" ,
//						*argv );
					}
				unlink( oname );	// spooler has it

				if ( c == EOF )
					goto nextfile ;
				goto newframe ;
			case `D':	/* lower pen */
				drawn = NO ;	// ensure point
				virpen = `D' ;
				continue ;

			case `U':	/* raise pen */
				virpen = `U' ;	// just keep track
				continue ;

			case `C':	/* change color */
				if ( ! drawn )
					{
					buildvec( &virpos , &virpos );
					plotted = drawn = YES ;
					}
#ifdef	COLORPLOT
				if ( fread( &color , sizeof color ,
				1 , stdin ) != 1 )
					continue ;	// EOF
				if ( color < WHITE || color > BLACK )
					bomb( "bad color in %s" , *argv );
#else
				getchar();	// ignore even if EOF
				getchar();
#endif
				continue ;
			case `M':	/* move pen */
				if ( fread( &newpos , sizeof newpos ,
				1 , stdin ) != 1 )
					continue ;	// EOF

				// convert to Trilog pixels
				newpos.x = ((long)newpos.x*XPI + 500) / 1000 ;
				if ( newpos.x > XMAX )
					newpos.x = XMAX ;
				newpos.y = ((long)newpos.y*YPI + 500) / 1000 ;
				if ( newpos.y > YMAX )
					newpos.y = YMAX ;

				if ( virpen == `U' )
					{
					if ( ! drawn )
						{
						buildvec( &virpos , &virpos );
						plotted = drawn = YES ;
						}
					}
				else	{	// draw the stroke
					buildvec( &virpos , &newpos );
					plotted = drawn = YES ;
					}
				virpos = newpos ;	// update location
				continue ;

			default:
				bomb( "bad input in %s" , *argv );
				}

    nextfile:	;
		} while ( argc > 1 );

	exit( 0 );
	}
/*
	initfile - initialize page image disk file
*/


static
initfile()
	{
	register char		*rp ;	// -> buffer[.][.]
	register unsigned	b ;	// band index
	
	// create the file
	if ( (ofp = fopen( oname , "w" )) == NULL )
		bomb( "can't create %s" , oname );

	// create an empty band image in active-band buffer:
	for ( rp = &buffer[0][0] ; rp < &buffer[XCHUNK][0] ; )
		{
		register unsigned	byte ;

		for ( byte = 0 ; byte < BYTES ; ++ byte )
			*rp++ = BIT6 ;	// no dots
		*rp++ = PLOTMODE ;
		*rp++ = `\n' ;
		}

	// write all bands to disk:
	for ( b = 0 ; b < NBANDS ; ++ b )
		if ( fwrite( &buffer[0][0] , sizeof buffer , 1 , ofp ) != 1 )
			bomb( "can't initialize %s" , oname );

	fclose( ofp );
	}
/*
	buildvec - set up DDA parameters and queue vector
*/


static
buildvec( pt1 , pt2 )
	coords		*pt1 ;		// endpoints
	coords		*pt2 ;
	{
	register vect	*vp ;		// -> vector rasterization descriptor

	// arrange for pt1 to have smaller X-coordinate:
	if ( pt1->x > pt2->x )
		{
		register coords	*temp ;

		temp = pt1 ;		// swap pointers
		pt1 = pt2 ;
		pt2 = temp ;
		}

	vp = allocate();		// allocate a descriptor

	// set up multi-band DDA parameters for stroke
#ifdef	COLORPLOT
	vp->colors = color ;		// draw vector in current color
#endif
	vp->nib = *pt1 ;		// initial pixel
	vp->major = pt2->x - vp->nib.x ;	// always nonnegative
	vp->xsign = vp->major ? 1 : 0 ;
	vp->minor = (int)pt2->y - (int)vp->nib.y ;
	vp->ysign = vp->minor ? (vp->minor > 0 ? 1 : -1) : 0 ;
	if ( vp->ysign < 0 )
		vp->minor = - vp->minor ;

	// if X is not really major, correct the assignments
	if ( ! (vp->xmajor = vp->minor <= vp->major) )
		{
		register int	temp ;

		temp = vp->minor ;
		vp->minor = vp->major ;
		vp->major = temp ;
		}

	vp->e = vp->major / 2 - vp->minor ;	// initial DDA error
	vp->de = vp->major - vp->minor ;

	// link descriptor into band corresponding to starting X nib
	queue( &band[(int)vp->nib.x / XCHUNK] , vp );
	}
/*
	allocate - allocate a descriptor (possibly updating disk file)
*/


static vect	*
allocate()				// returns address of descriptor
	{
	register vect	*vp ;

	vp = malloc( sizeof(vect) );
	if ( vp == NULL )
		{
		outbuild();		// flush and free up storage
		vp = malloc( sizeof(vect) );
		if ( vp == NULL )
			bomb( "BUG: malloc failed" , NULL );
		}
	return ( vp );
	}
/*
	queue - queue descriptor onto band list

	Note that descriptor order is not important.
*/


static
queue( hp , vp )
	register vect	**hp;		// *hp -> first descriptor in list
	register vect	*vp ;		// -> new descriptor
	{
#ifdef	DEBUG
	if ( hp == NULL )
		bomb( "BUG: null hp in queue" , NULL );
	if ( vp == NULL )
		bomb( "BUG: null vp in queue" , NULL );
#endif
	vp->nextv = *hp ;		// -> first in existing list
	*hp = vp ;			// -> new first descriptor
	}
/*
	dequeue - remove descriptor from band list (do not free space)
*/


static vect	*
dequeue( hp )
	register vect	**hp;		// *hp -> first descriptor in list
	{
	register vect	*vp ;

#ifdef	DEBUG
	if ( hp == NULL )
		bomb( "BUG: null hp in dequeue" , NULL );
#endif
	vp = *hp ;
	if ( vp != NULL )
		*hp = vp->nextv ;	// -> next element in list
	return ( vp );			// may be NULL
	}
/*
	outbuild - rasterize all vectors into disk page image
*/


static
outbuild()
	{
	register vect	**hp;		// *hp -> head of descriptor list
	register vect	*vp ;		// -> rasterization descriptor
	register vect	**np;		// `hp' for next band

	for ( hp = &band[0] ; hp < &band[NBANDS] ; ++ hp )
		if ( *hp != NULL )
			break ;
	if ( hp == &band[NBANDS] )
		return ;		// nothing to do

	rename( oname , iname );
	if ( (ifp = fopen( iname , "r" )) == NULL )
		bomb( "can't open %s" , iname );
	if ( (ofp = fopen( oname , "w" )) == NULL )
		bomb( "can't recreate %s" , oname );

	for ( hp = &band[0] , np = &band[1] , xstart = 0 ;
	      hp < &band[NBANDS] ; hp = np++ , xstart += XCHUNK )
		{
		// roll the band's disk image into active-band buffer
		if ( fread( &buffer[0][0] , sizeof buffer , 1 , ifp ) != 1 )
			bomb( "can't read %s" , iname );

		while ( (vp = dequeue( hp )) != NULL )
			raster( vp , np );	// rasterize vector
		// "raster" either re-queued the descriptor onto the
		// next band list or else it freed the descriptor

		// roll the band back out to disk
		if ( fwrite( &buffer[0][0] , sizeof buffer , 1 , ofp ) != 1 )
			bomb( "can't write %s" , oname );
		}

	fclose( ifp );
	fclose( ofp );
	unlink( iname );
	}
/*
	rename - rename file by linking
*/


static
rename( from , to )
	register char	*from ;		// old name
	register char	*to ;		// new name
	{
//	unlink( to );			// not needed here
	if ( link( from , to ) != 0 )
		bomb( "link to %s failed" , from );
	unlink( from );
	}
/*
	optimize - scrunch final disk file to save printer time

Method:
	While copying the file, truncate each scan at the rightmost
	pixel.  Save up blank scans; dump them when a non-empty scan
	is seen (this avoids tail-of-page spacing).
	This doesn't work on an Okidata printer since it doesn't
	clear the previous scan's data from the trailer.
*/


static
optimize()
	{
#ifndef	COLORPLOT
#ifndef	OKIDATA
	register int	len ;		// useful plot bytes in scan
	register int	nbs ;		// # of held-back blank scans
	unsigned	scan ;		// counts plot scans

	rename( oname , iname );
	if ( (ifp = fopen( iname , "r" )) == NULL )
		bomb( "can't open %s" , iname );
	if ( (ofp = fopen( oname , "w" )) == NULL )
		bomb( "can't recreate %s" , oname );

	for ( scan = nbs = 0 ; scan < NSCANS ; ++ scan )
		{
		if ( fread( &buffer[0][0] , BYTES+2 , 1 , ifp ) != 1 )
			bomb( "error reading %s" , iname );
		for ( len = BYTES-1 ; len >= 0 ; -- len )
			if ( buffer[0][len] != BIT6 )
				break ;
		if ( ++len > 0 )
			{
			for ( ; nbs > 0 ; -- nbs )
				{
				putc( PLOTMODE , ofp );
				putc( `\n' , ofp );
				}
			if ( fwrite( &buffer[0][0] , len , 1 , ofp ) != 1 )
				bomb( "error writing %s" , oname );
			putc( PLOTMODE , ofp );
			putc( `\n' , ofp );
			}
		else
			++ nbs ;	// defer blank line
		}
	fclose( ifp );
	fclose( ofp );
	unlink( iname );
#endif
#endif
	}
/*
	raster - rasterize vector.  If overflow into next band, requeue;
					else free the descriptor.

Method:
	Modified Bresenham algorithm.
*/


static
raster( vp , np )
	register vect		*vp ;	// -> vector rasterization descriptor
	vect			**np;	// *np -> next band's first descr
	{
	register unsigned	dx ;	// raster within active band

	for ( dx = vp->nib.x - xstart ; dx < XCHUNK ; )
		{
		putdot( dx , vp->nib.y );	// ensure at least a dot
		if ( vp->major-- == 0 )	// done!
			{
			free( vp );	// return to "malloc" package
			return ;
			}
		if ( vp->e < 0 )	// advance major & minor
			{
			dx += vp->xsign ;
			vp->nib.y += vp->ysign ;
			vp->e += vp->de ;
			}
		else	{		// advance major only
			if ( vp->xmajor )	// X is major direction
				++ dx ;
			else			// Y is major direction
				vp->nib.y += vp->ysign ;
			vp->e -= vp->minor ;
			}
		}

	// overflow into next band; re-queue
	vp->nib.x = xstart + XCHUNK ;
	queue( np , vp );		// DDA parameters already set
	}
/*
	putdot - set pixel in active-band buffer
*/


static
putdot( relx , absy )
	register unsigned	relx ;	// relative to `xstart'
	register unsigned	absy ;
	{
#ifdef	DEBUG
	if ( relx >= XCHUNK )
		bomb( "BUG: putdot relx=%u" , relx );
	if ( absy > YMAX )
		bomb( "BUG: putdot absy=%u" , absy );
#endif
	buffer[relx][(int)absy / BITS] |= 1 << (int)absy % BITS ;
	}
/*
	bomb - print fatal error message then die
*/


static
bomb( fmt , val )
	register char	*fmt ;		// format for "fprintf"
	register char	*val ;		// possible value for "fprintf"
	{
	fputs( "\07* tri * " , stderr );
	fprintf( stderr , fmt , val );
	putc( `\n' , stderr );

	unlink( iname );
	unlink( oname );

	exit( 1 );
	}
/*
	onintr - invoked on interrupt
*/


static
onintr()
	{
	signal( SIGINT , SIG_IGN );
	signal( SIGQUIT , SIG_IGN );
	signal( SIGTERM , SIG_IGN );	// yes, there IS a race here
	bomb( "interrupted" , NULL );
	}
