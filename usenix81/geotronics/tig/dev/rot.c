//#define	DEBUG	yes
/*
 * rot.c - TIG-pack rotation, scaling, translation, and clipping filter
 *
 *	last edit: 01-Dec-1980	D A Gwyn
 *
 * Command format is
 *	rot[ deg[ xc yc]][ -s xf[ yf]][ -t xo yo][ -c xl yb xr yt]
 * Input is a TIG device-independent plot file on the standard input.
 * Output is TIG code on the standard output, which is normally piped
 * into a TIG device interpreter.
 *
 * Coords are in device inches.  Operations are applied in this order:
 *	1) Rotate image by deg degrees CCW about (xc,yc) [(0,0) default];
 *	2) Scale image by (xf,yf) [yf=xf if not specified] about (xc,yc);
 *	3) Translate (offset) image by (xo,yo).
 *	4) Clip resulting coords to first quadrant H/V rectangle whose
 *	   LLC=(xl,yb) and URC=(xr,yt).
 *
 * For strange clipping, you can pipe through two "rot" filters.
 * "rot" is also useful as a pen-motion optimizer for random vectors.
 *
 * Compile:
 *	# cc -s -n -O rot.c -lm -o /usr/bin/rot
 *	# chmod 755 /usr/bin/rot
 *
 * Examples:
 *	% main.out | rot -s 0.3 -c 0 0 6 5 | tee rgout | rg
 *		previews H.I. plotter output on RG-512 terminal and
 *		puts a copy in "rgout" for later	% rg <rgout
 *	% rot 90 -t 5 0 <rgout | tri
 *		prints the same file on the Trilog "right side up"
 */

#include	<stdio.h>
#include	<signal.h>
#include	<math.h>
#include	<ctype.h>

#define DEGRAD	57.2957795		/* degrees/radian */

struct	coords
	{
	double		x ;
	double		y ;
	};

static struct coords	actpos ;	/* output pen position */
static struct coords	virpos ;	/* virtual pen position */
static int		actpen ;	/* output pen state */
static int		virpen ;	/* virtual pen state */

struct	ucoords
	{
	unsigned	ux ;
	unsigned	uy ;
	};

static struct ucoords	lastout ;	/* TIG coord output buffer */
static struct coords	newpos ;	/* converted input position */
static struct coords	llc ;		/* clipping window LLC */
static struct coords	urc ;		/* clipping window URC */


static
onintr()
	{
	signal( SIGINT , SIG_IGN );
	signal( SIGQUIT , SIG_IGN );
	signal( SIGTERM , SIG_IGN );	/* yes, there IS a race here */
	error( "interrupted" , 0 );
	}


main( argc , argv )
	int		argc ;
	char		*argv[];
	{
	char		ch ;
	double		a , b , c , cost , d , e , f , sint , theta ;
	struct coords	center , scale , offset ;
	struct ucoords	input ;		/* TIG coord input buffer */

	if ( isatty( fileno( stdin ) ) )
		error( "missing input" , argc == 1 );
	if ( isatty( fileno( stdout ) ) )
		error( "missing output" , argc == 1 );

	signal( SIGHUP , SIG_IGN );
	if ( signal( SIGINT , SIG_IGN ) != SIG_IGN )
		signal( SIGINT , &onintr );
	if ( signal( SIGQUIT , SIG_IGN ) != SIG_IGN )
		signal( SIGQUIT , &onintr );
	signal( SIGTERM , &onintr );

	/* set default argument values */
	scale.x = scale.y = 1.0 ;
	theta = center.x = center.y = offset.x = offset.y = 0.0 ;
	llc.x = llc.y = 0.0 ;
	urc.x = urc.y = 65535.0 ;	/* TIG maximum */

	/* process arguments; replace defaults */
#define	ispn(c)	(isdigit( c ) || c == '.')
#define	isopt()	((ch = (*++argv)[0]) == '-' && ! ispn( (*argv)[1] ))
#define	isbad()	(ch != '-' && ! ispn( ch ) )
#define	nonum()	(--argc <= 0 || isopt() || isbad())
#define	tigs(p)	(atof( p ) * 1000.0)
	while ( --argc > 0 )
		if ( isopt() )
			switch ( (*argv)[1] )
				{
			case 's':	/* scale factors */
				if ( nonum() )
					error( "bad xf" , 1 );
				scale.y = scale.x = atof( *argv );
				if ( --argc <= 0 )
					continue ;
				if ( isopt() )
					{
					++ argc ;	/* rescan */
					-- argv ;
					continue ;
					}
				if ( isbad() )
					error( "bad yf" , 1 );
				scale.y = atof( *argv );
				continue ;
			case 't':	/* translation offset */
				if ( nonum() )
					error( "bad xo" , 1 );
				offset.x = tigs( *argv ) ;
				if ( nonum() )
					error( "bad yo" , 1 );
				offset.y = tigs( *argv ) ;
				continue ;
			case 'c':	/* clipping window corners */
				if ( nonum()
				|| (llc.x = tigs( *argv )) < 0.0 )
					error( "bad xl" , 1 );
				if ( nonum()
				|| (llc.y = tigs( *argv )) < 0.0 )
					error( "bad yb" , 1 );
				if ( nonum()
				|| (urc.x = tigs( *argv )) <= llc.x )
					error( "bad xr" , 1 );
				if ( nonum()
				|| (urc.y = tigs( *argv )) <= llc.y )
					error( "bad yt" , 1 );
				continue ;
			default:
				error( "bad option" , 1 );
				}
		else	{
			if ( isbad() )
				error( "bad deg" , 1 );
			theta = atof( *argv ) / DEGRAD ;
			if ( --argc <= 0 )
				break ;
			if ( isopt() )
				{
				++ argc ;	/* rescan */
				-- argv ;
				continue ;
				}
			if ( isbad() )
				error( "bad xc" , 1 );
			center.x = tigs( *argv ) ;
			if ( nonum() )
				error( "bad yc" , 1 );
			center.y = tigs( *argv ) ;
			continue ;
			}
#undef	ispn
#undef	isopt
#undef	isbad
#undef	nonum
#undef	tigs
#ifdef	DEBUG
	fprintf( stderr , "theta=%f xc=%f yc=%f\nxf=%f yf=%f\n" ,
		theta , center.x , center.y , scale.x , scale.y );
	fprintf( stderr , "xo=%f yo=%f\nxl=%f yb=%f xr=%f yt=%f\n" ,
		offset.x , offset.y , llc.x , llc.y , urc.x , urc.y );
#endif

	/* build transformation matrix */
	cost = cos( theta );
	sint = sin( theta );
	a = scale.x * cost ;
	b = (- scale.x) * sint ;
	c = center.x + offset.x -
		scale.x * (center.x * cost - center.y * sint);
	d = scale.y * sint ;
	e = scale.y * cost ;
	f = center.y + offset.y -
		scale.y * (center.y * cost + center.x * sint);
#ifdef	DEBUG
	fprintf( stderr , "a=%f b=%f c=%f\nd=%f e=%f f=%f\n" ,
		a , b , c , d , e , f );
#endif

	/* process the file */
    newframe:
	lastout.ux = lastout.uy = 0 ;
	virpos.x = actpos.x = virpos.y = actpos.y = 0.0 ;
	virpen = actpen = 'U' ;

	while ( (ch = getchar()) != EOF )
		{
		switch ( ch )
			{
		case 'F':		/* new frame */
			putchar( ch );
			fflush( stdout );	/* for terminals */
			goto newframe ;
		case 'D':		/* lower pen */
			/* `offset' is available for temp use */
			offset.x = newpos.x = virpos.x ;
			offset.y = newpos.y = virpos.y ;
			virpen = 'D';
			goto mark ;	/* make a dot at least */
		case 'U':		/* raise pen */
			virpen = 'U' ;
			continue ;
		case 'C':		/* change color */
			putchar( ch );
			if ( (ch = getchar()) != EOF )
				putchar( ch );
			if ( (ch = getchar()) != EOF )
				putchar( ch );
			continue ;
		case 'M':		/* move pen */
			if ( fread( &input , sizeof(input), 1 , stdin ) != 1 )
				continue ;
			/* `center', `scale', `offset' are unused */
			scale.x = (double)input.ux ;
			scale.y = (double)input.uy ;
			offset.x = newpos.x =
					a * scale.x + b * scale.y + c ;
			offset.y = newpos.y = 
					d * scale.x + e * scale.y + f ;
			/* clip coordinates to rectangular window */
    mark:		if ( virpen == 'D' && clip() )
				line();	/* updates `actpos' */
			virpos = offset ;
			continue ;
		default:
			error( "bad input data" , 0 );
			}
		}

	exit( 0 );
	}


static int
clip()				/* clips virpos..newpos */
	{
	register struct coords	*p0 , *p1 ;
	register int		c ;
	int			c0 , c1 ;
	double			x , y ;

	/* modified Cohen-Sutherland algorithm */

#define LEFT	1
#define RIGHT	2
#define BOTTOM	4
#define TOP	8

#define	code(p)	((p->x < llc.x ? LEFT : (p->x > urc.x ? RIGHT : 0)) \
		| (p->y < llc.y ? BOTTOM : (p->y > urc.y ? TOP : 0)))

	p0 = &virpos ;
	c0 = code( p0 );
	p1 = &newpos ;
	c1 = code( p1 );

#define	slide(u,v,nam)	{ u = p0->u + (p1->u - p0->u) * (p0->v - nam.v) \
			/ (p0->v - p1->v); v = nam.v; }

	while ( c0 || c1 )
		{
		if ( c0 & c1 )
			return ( 0 );	/* (vector is garbage) */
		c = c0 ? c0 : c1 ;
		if ( c & LEFT )
			slide( y , x , llc )
		else if ( c & RIGHT )
			slide( y , x , urc )
		else if ( c & BOTTOM )
			slide( x , y , llc )
		else	/* c & TOP */
			slide( x , y , urc )
		if ( c == c0 )
			{
			p0->x = x;
			p0->y = y;
			c0 = code( p0 );
			}
		else	/* c == c1 */
			{
			p1->x = x;
			p1->y = y;
			c1 = code( p1 );
			}
		}
	return ( 1 );			/* vector visible */
	}
#undef	code
#undef	slide


static
pendown()				/* drop pen */
	{
	if ( actpen == 'D' )
		return ;
	putchar( 'D' );
	actpen = 'D';
	}


static
penup()					/* lift pen */
	{
	if ( actpen == 'U' )
		return ;
	putchar( 'U' );
	actpen = 'U';
	}


static
line()					/* draw virpos..newpos */
	{
	register struct coords	*p0 , *p1 , *tp ;
	struct ucoords		u0 , u1 ;

	p0 = &virpos ;
	p1 = &newpos ;

	/* optimize pen motion */
#define	abs(x)		((x) < 0 ? -(x) : (x))
#define	dist(dx,dy)	(abs( dx ) > abs( dy ) ? abs( dx ) : abs( dy ))
	if ( dist( p0->x - actpos.x , p0->y - actpos.y )
	   > dist( p1->x - actpos.x , p1->y - actpos.y ) )
		{			/* swap points */
		tp = p0 ;
		p0 = p1 ;
		p1 = tp ;
		}
#undef	abs
#undef	dist

	/* convert (round) to TIG coordinates */
	u0.ux = (unsigned)(p0->x + 0.5);
	u0.uy = (unsigned)(p0->y + 0.5);
	actpos.x = (double)(u1.ux = (unsigned)(p1->x + 0.5));
	actpos.y = (double)(u1.uy = (unsigned)(p1->y + 0.5));
	/* maintainers beware - that one was a bit tricky! */

	if ( u0.ux != lastout.ux || u0.uy != lastout.uy )
		{
		penup();
		move( &u0 );		/* move to start point */
		}
	pendown();			/* guarantee a dot at least */
	if ( u1.ux != lastout.ux || u1.uy != lastout.uy )
		move( &u1 );		/* draw to end point */
	}


static
move( p )
	register struct ucoords	*p ;
	{
	putchar( 'M' );
	fwrite( p , sizeof(*p), 1 , stdout );
	lastout = *p ;			/* remember where we've been */
	}


static
error( msg , usage )			/* display fatal error */
	char	*msg ;
	int	usage ;			/* nonzero => instructions */
	{
	fprintf( stderr , "\07* rot * %s *\n" , msg );
	if ( usage )
		fprintf( stderr ,
"Usage: rot[ deg[ xc yc]][ -s xf[ yf]][ -t xo yo][ -c xl yb xr yt]\n" );
	exit( 1 );
	}
