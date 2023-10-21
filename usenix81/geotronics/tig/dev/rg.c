/*
 * rg.c - TIG-pack device interpreter for RG-512 terminal on UNIX
 *
 *	last edit: 14-Jun-1981	D A Gwyn
 *
 * This routine interprets TIG device-independent codes and outputs
 * RG-512 graphic sequences to its standard output.  Operator input
 * via stderr controls newframe action, as described below.
 * The RG-512 display is considered 6 inches wide and 5 inches high with
 * 512x240 pixels to maintain compatibility with MTX512 plotting
 * programs (there are actually 512x250 pixels).
 * Edge-limiting is done here; use "rot" if clipping is required.
 * A near-optimal graphic character sequence is generated.
 *
 * Compile:
 *	# cc -s -n -O rg.c -o /usr/bin/rg
 * or (for buffered output -- reduce baud rate to 2400 max)
 *	# cc -s -n -O -DBUFFERED rg.c -o /usr/bin/rg
 *	# chmod 755 /usr/bin/rg
 *
 * Usage:
 *	% rg input_file_names
 * or
 *	% rg < input
 * or
 *	% main.out | rg
 * or (to clip)
 *	% main.out | rot -c 0 0 6 5 | rg
 * or (to save display sequence in a file for later	% cat save.rg)
 *	% main.out | rg >save.rg
 * (This may be run on a non RG-512 terminal.)
 *
 * If outputing to a terminal, then upon opening a new file or reading
 * a newframe code the program beeps stderr and waits for an input
 * line on stderr before clearing the screen.  A similar pause occurs
 * after the last file unless the "-q" flag was given, in which case an
 * immediate exit is taken with the display still up.
 */

#include	<std.h>
#include	<signal.h>
#include	<stdio.h>

extern char	_sobuf[BUFSIZ] ;	/* stdout buffer */

struct	coords
	{
	unsigned	x ;
	unsigned	y ;
	};

static struct coords	virpos ;	/* virtual pen position */
static struct coords	actpos ;	/* beam follower */

static char	hix , lox , hiy , loy ;	/* last output coords */

static bool	frames = NO ;		/* second or later frame */

static bool	term = NO ;		/* YES => stdout is tty */

static bool	qflag = NO ;		/* YES => "-q" option */

static bool	drawn = YES ;		/* NO => point needed */

/* device parameters */
#define	XPPI	256/3
#define	YPPI	48			/* MTX512 compatible */
#define	XMAX	511
#define	YMAX	249

#define	ALF	'\037'
#define	VCT	'\035'
#define	PNT	'\034'
static char	mode = ALF ;		/* RG-512 mode */

#define	CLRALF	'\032'
#define	CLRVCT	'\031'


static
onintr()				/* interrupt handler */
	{
	signal( SIGINT , SIG_IGN );
	signal( SIGQUIT , SIG_IGN );
	signal( SIGTERM , SIG_IGN );	/* yes, there IS a race here */
	bomb( "interrupted" , NULL );
	}


static
bomb( fmt , arg )			/* fatal error message */
	register char	*fmt , *arg ;
	{
	if ( term )
		freopen( "/dev/tty" , "w" , stdout );
	clear();
	alfmode();
	fflush( stdout );
	freopen( "/dev/tty" , "w" , stderr );	/* just in case */
	fprintf( stderr , "\07* rg * " );
	fprintf( stderr , fmt , arg );
	fprintf( stderr , " *\n" );
	exit( 1 );
	}


static
clear()					/* clear screen on stdout */
	{
	vctmode();
	putchar( CLRVCT );
	fflush( stdout );
	if ( term )
		sleep( 1 );		/* let settle */
	actpos.x = actpos.y = (unsigned)(-1);	/* position lost */
	}


static
clrtext()				/* clear text (stdout is tty) */
	{
	alfmode();
	putchar( CLRALF );
	}


main( argc , argv )
	int		argc ;
	char		*argv[];
	{
	register char	c ;		/* input character */
	char		resp[2] ;	/* operator input buffer */
	register int	virpen ;	/* virtual pen state */
	struct coords	input ;		/* current input coords */

	if ( signal( SIGINT , SIG_IGN ) != SIG_IGN )
		signal( SIGINT , &onintr );
	if ( signal( SIGQUIT , SIG_IGN ) != SIG_IGN )
		signal( SIGQUIT , &onintr );
	signal( SIGPIPE , &onintr );
	signal( SIGTERM , &onintr );

	if ( argc < 2 && isatty( fileno( stdin ) ) )
		{
		fclose( stdout );	/* prevent clear() etc. */
		bomb( "missing input" , NULL );
		}

	if ( qflag = strcmp( *++argv , "-q" ) == 0 )
		-- argc ;
	else
		-- argv ;

	argv[0] = "stdin";		/* for "bomb" message */
	term = isatty( fileno( stdout ) ) ;
#ifdef	BUFFERED
	setbuf( stdout , _sobuf );
#endif

	clrtext();

	do	{
		/* if no file specs, use standard input */
		if ( --argc > 0 && freopen( *++argv , "r" , stdin ) == NULL )
			{
			if ( term )
				{
				clear();	/* not clrtext() */
				alfmode();
				fflush( stdout );
				}
			fprintf( stderr , "\07* rg * \"%s\" not found *\n" ,
			*argv );
			/* fall through to newframe */
			}

    newframe:	if ( term && frames )
			{
			/* do not clrtext() */
			alfmode();
			putchar( '\07' );	/* beep */
			fflush( stdout );
			freopen( "/dev/tty" , "r" , stderr );	/* yuck! */
			fgets( resp , 2 , stderr );
			freopen( "/dev/tty" , "w" , stderr );
			clear();
			clrtext();
			}
		else	{
			frames = YES ;
			clear();	/* non-interactive */
			}

		virpos.x = virpos.y = 0 ;
		virpen = 'U' ;
		actpos.x = actpos.y = (unsigned)(-1) ;	/* force move */

		while ( (c = getchar()) != EOF )
			switch( c )
				{
			case 'F':	/* new frame */
				drawpt();
				goto newframe ;

			case 'D':	/* lower pen */
				drawn = NO ;
				virpen = 'D' ;
				continue ;

			case 'U':	/* raise pen */
				virpen = 'U' ;
				continue ;

			case 'C':	/* change color */
				/* drawpt(); */
				if ( getchar() == EOF )
					goto nextfile ;
				if ( getchar() == EOF )
					goto nextfile ;
				continue ;

			case 'M':	/* move pen */
				if ( fread( &input , sizeof(input) , 1 ,
				stdin ) != 1 )
					goto nextfile ;

				/* convert to RG-512 pixels */
				input.x = ((long)input.x*XPPI + 500) / 1000 ;
				if ( input.x > XMAX )
					input.x = XMAX ;
				input.y = ((long)input.y*YPPI + 500) / 1000 ;
				if ( input.y > YMAX )
					input.y = YMAX ;

				if ( virpen == 'U' )	/* update position */
					{
					drawpt();
					virpos = input ;
					continue ;
					}

				/* must draw - get to start point of stroke */
				if ( mode != VCT
				|| virpos.x != actpos.x
				|| virpos.y != actpos.y )
					{
					mode = ALF ;	/* fake */
					vctmode();
					move();	/* dark vector */
					}

				/* draw the stroke */
				actpos.x = virpos.x = input.x ;
				actpos.y = virpos.y = input.y ;
				move();	/* bright vector */
				drawn = YES ;
				continue ;

			default:
				bomb( "bad data in %s" , *argv );
				}

    nextfile:	drawpt();
		} while ( argc > 1 );

	if ( ! qflag )
		{
		if ( term )
			{
			/* do not clrtext() */
			alfmode();
			putchar( '\07' );	/* beep */
			fflush( stdout );
			freopen( "/dev/tty" , "r" , stderr );	/* yuck! */
			fgets( resp , 2 , stderr );
			freopen( "/dev/tty" , "w" , stderr );
			clrtext();
			}
		clear();
		}
	alfmode();
	exit( 0 );
	}


static
drawpt()				/* put point if necessary */
	{
	if ( drawn )
		return ;
	pntmode();
	move();				/* bright point */
	actpos = virpos ;
	drawn = YES ;
	}


/* RG-512 code bits (same as Tektronix 4010) */
#define	HIXBIT	0040
#define	LOXBIT	0100
#define	HIYBIT	0040
#define	LOYBIT	0140


static
move()					/* move beam to virpos */
	{
	register char	c ;

	if ( (c = virpos.y >> 5) != hiy )
		putchar( (hiy = c) | HIYBIT );
	if ( (c = virpos.x >> 5) != hix )
		{
		putchar( (loy = virpos.y & 037) | LOYBIT );
		putchar( (hix = c) | HIXBIT );
		}
	else if ( (c = virpos.y & 037) != loy )
		putchar( (loy = c) | LOYBIT );
	putchar( (lox = virpos.x & 037) | LOXBIT );
	}


static
alfmode()				/* put in alpha mode */
	{
	if ( mode != ALF )
		putchar( mode = ALF );
	}


static
vctmode()				/* put in vector mode */
	{
	if ( mode == VCT )
		return ;
	putchar( mode = VCT );
	hix = lox = hiy = loy = -1 ;	/* lost position */
	/* note that the next vector will be dark */
	}


static
pntmode()				/* put in point mode */
	{
	if ( mode == PNT )
		return ;
	putchar( mode = PNT );
	hix = lox = hiy = loy = -1 ;	/* lost position */
	}
