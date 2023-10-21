/*
	hi -- TIG-pack device interpreter for H.I. plotter on UNIX

	last edit: 09-Feb-1981	D A Gwyn

Function:
	This routine interprets TIG device-independent codes and plots
	on the Houston Instruments Complot DP-3 pen plotter, which has
	200 plotter increments per inch; the display surface is 22
	inches high and longer than the TIG coordinate range.  Clipping
	is not done here since /dev/xy takes care of such things.
	Direct device access is required since spooling uses up disk.

Compile:
	# cc -s -n -O hi.c -o /usr/bin/hi
	# chmod 755 /usr/bin/hi

Usage:
	% hi input_file_names
		or
	% hi < input
		or
	% main.out | hi
*/

#include	<signal.h>
#include	<stdio.h>

/* device parameters */
#define	PPI	200			/* points per inch */

#define	PLOTTER	"/dev/xy"


static
onintr()				/* interrupt handler */
	{
	signal( SIGINT , SIG_IGN );
	signal( SIGQUIT , SIG_IGN );
	signal( SIGTERM , SIG_IGN );	// yes, there IS a race here
	bomb( "interrupted" , NULL );
	}


struct	coords
	{
	unsigned	x ;
	unsigned	y ;
	};

static struct coords	actpos ;	// actual pen position
static struct coords	virpos ;	// virtual pen position
static int		actpen ;	// actual pen state


main( argc , argv )
	int		argc ;
	char		*argv[];
	{
	int		c ;		// input character
	int		virpen ;	// virtual pen state
	struct coords	input ;		// current input coordinates
	int		warned = 0 ;	// for timing messages

	if ( argc < 2 && isatty( fileno( stdin ) ) )
		bomb( "missing input" , NULL );
	argv[0] = "stdin";		// for "bomb" message

	signal( SIGHUP , SIG_IGN );
	if ( signal( SIGINT , SIG_IGN ) != SIG_IGN )
		signal( SIGINT , &onintr );
	if ( signal( SIGQUIT , SIG_IGN ) != SIG_IGN )
		signal( SIGQUIT , &onintr );
	signal( SIGTERM , &onintr );

	while ( freopen( PLOTTER , "w" , stdout ) == NULL )
		{
		if ( warned++ % 40 == 0 )	// every 10 minutes
			fprintf( stderr ,
			"* hi * waiting for %s...\n" , PLOTTER );
		putc( `\07' , stderr );	// beep every quarter minute
		sleep( 15 );
		}
	if ( warned )
		fputs( "\07* hi * starting plot\n" , stderr );

	do	{
		// if no file specs, use standard input
		if ( --argc > 0 && freopen( *++argv , "r" , stdin ) == NULL )
			{
			fprintf( stderr , "\07* hi * \"%s\" not found *\n" ,
			*argv );
			goto nextfile ;
			}

    newframe:	page();			// updates actual parameters
		virpos.x = virpos.y = 0 ;
		virpen = `U' ;

		for ( ; ; )
			switch( c = getchar() )
				{
			case EOF:
				goto nextfile ;

			case `F':	/* new frame */
				goto newframe ;

			case `D':	/* lower pen */
				if ( virpos.x != actpos.x
				  || virpos.y != actpos.y )
					{
					penup();
					move();
					}
				pendown();	// make a dot
				virpen = `D' ;
				continue ;

			case `U':	/* raise pen */
				virpen = `U' ;	// just keep track
				continue ;

			case `C':	/* change color */
				getchar();	// ignore even if EOF
				getchar();
				continue ;

			case `M':	/* move pen */
				if ( fread( &input , sizeof(input) , 1 ,
				stdin ) != 1 )
					continue ;	// EOF

				// convert to plotter increments
				input.x = ((long)input.x*PPI + 500) / 1000 ;
				input.y = ((long)input.y*PPI + 500) / 1000 ;

				if ( virpen == `U' )	// update position
					{
					virpos = input ;
					continue ;
					}

				// must draw - get to start point of stroke
				if ( virpos.x != actpos.x
				  || virpos.y != actpos.y )
					{
					penup();
					move();
					pendown();
					}

				// draw the stroke
				virpos = input ;
				move();
				continue ;

			default:
				bomb( "bad data in %s" , *argv );
				}

    nextfile:	;
		} while ( argc > 1 );

	exit( 0 );
	}


/* /dev/xy data bits -- may be ORed together */
#define	POSX		0001
#define	NEGX		0002
#define	POSY		0004
#define	NEGY		0010
#define	PENDOWN		0020
#define	PENUP		0040
#define	FORM		0200


static
page()					/* advance to fresh page */
	{
	putchar( FORM );
	actpos.x = actpos.y = 0 ;
	actpen = `U' ;
	}


static
pendown()				/* drop pen */
	{
	if ( actpen == `D' )
		return ;
	putchar( PENDOWN );
	actpen = `D' ;
	}


static
penup()					/* lift pen */
	{
	if ( actpen == `U' )
		return ;
	putchar( PENUP );
	actpen = `U' ;
	}


static
move()					/* make actual pos = virtual pos */
	{
	register char	c ;		// composite control code
	char		majchar , minchar ;	// sorted bits
	char		xchar , ychar ;	// device control bits
	long		de ;		// DDA jag amount
	long		e ;		// DDA error accumulator
	long		major , minor ;	// sorted deltas
	long		xdelta , ydelta ;	// distance to travel

	/* note that the algorithm works for all special cases, too */

	// determine distance and direction of X and Y travels
	if ( (xdelta = (long)virpos.x - (long)actpos.x) < 0 )
		{
		xdelta = - xdelta ;
		xchar = NEGX ;
		}
	else
		xchar = POSX ;
	if ( (ydelta = (long)virpos.y - (long)actpos.y) < 0 )
		{
		ydelta = - ydelta ;
		ychar = NEGY ;
		}
	else
		ychar = POSY ;

	if ( xdelta >= ydelta )
		{
		major = xdelta ;
		minor = ydelta ;
		majchar = xchar ;
		minchar = ychar ;
		}
	else	{
		major = ydelta ;
		minor = xdelta ;
		majchar = ychar ;
		minchar = xchar ;
		}

	/* modified Bresenham algorithm */
	e = major / 2 - minor ;		// error accumulator
	de = major - minor ;
	while ( major-- != 0 )
		{
		c = majchar ;		// major always steps
		if ( e < 0 )
			{
			c |= minchar ;	// minor sometimes does
			e += de ;
			}
		else
			e -= minor ;
		putchar( c );
		}

	actpos = virpos ;
	return ;
	}


static
bomb( fmt , arg )			/* fatal error message */
	register char	*fmt , *arg ;
	{
	fputs( "\07* hi * " , stderr );
	fprintf( stderr , fmt , arg );
	putc( `\n' , stderr );
	exit( 1 );
	}
