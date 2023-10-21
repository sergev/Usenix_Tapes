//#define	PLOT				// for "-v" support
/*
	prof -- print execution profile (compatible with V7 UNIX)

		-a option reports all symbols, not just externals
		-l option lists by symbol value rather than % time
		-v option outputs TIG graphic code
		-low option sets lowest % plotted
		-high option sets highest % plotted
		filename if specified replaces "a.out"

	last edit: 22-Feb-1981	D A Gwyn

"mon.out" file format:
	(unsigned) low PC being traced
	(unsigned) high PC being traced + 2
	(int) # traced procedures
	{
		(unsigned) entry PC + 8
		(long) # calls
	} [# traced procedures]
	{
		(unsigned) sample counter bin
	} [some number, usually dividing (high PC - low PC)]
*/

#include	<a.out.h>
#include	<ctype.h>
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>

static struct stat	statbuf ;
static struct exec	header ;
static struct nlist	entry ;

#define	MAXNAMES	1000
static struct nl			// special name-list for sorting
	{
	char			nl_name[8];
	unsigned		nl_value ;
	float			nl_time ;
	long			nl_ncall ;
	}		nl[MAXNAMES];
static int		nname ;		// # `nl' entries

#define	MAXCALLS	300
static struct cnt			// procedure call count record
	{
	unsigned		c_value ;
	long			c_ncall ;
	}		cbuf[MAXCALLS];
static int		ncount ;	// # `cbuf' entries

static unsigned 	aflg = 0 ;	// N_EXT if "-a" flag
static int		lflg = 0 ;
#ifdef	PLOT
static int		vflg = 0 ;
static int		low = 0 ;
static int		high = 100 ;
static int		vals = 0 ;
#endif
static char		*namfil = "a.out" ;	// exe file

static char		*monstr = "mon.out" ;	// profile data file

extern		exit(), l3tol();
extern char	*strncpy();
extern int	fstat();
#ifdef	PLOT
extern int	atoi();
#endif

static int	valcmp(), timcmp();
static		Error();
#ifdef	PLOT
static		Text(), Vector();
#endif

#define	min( a , b )	((a) < (b) ? (a) : (b))
#define	max( a , b )	((a) > (b) ? (a) : (b))

main( argc , argv )
	int			argc ;
	char			*argv[];
	{
	FILE			*pf ;
	int			bin ;
	unsigned		lowpc , highpc ;
	double			scale ;	// # bytes per sample bin
	double			totime , maxtime ;
	register struct nl	*np ;
	struct nl		*npe ;	// -> end of list

	while ( --argc > 0 )
		{
		if ( (*++argv)[0] == `-' )
#ifdef	PLOT
			if ( isdigit( (*argv)[1] ) )
				switch ( vals++ )
					{
				case 0:
					low = atoi( &(*argv)[1] );
					if ( low < 0 || low >= 100 )
						Error( "bad low arg" );
					break ;
				case 1:
					high = atoi( &(*argv)[1] );
					if ( high <= low || high > 100 )
						Error( "bad high arg" );
					break ;
				default:
					Error( "too many limit args" );
					}
			else
#endif
				switch ( (*argv)[1] )
					{
				case `a':
					aflg = N_EXT ;
					break ;
				case `l':
					++ lflg ;
					break ;
#ifdef	PLOT
				case `v':
					++ vflg ;
					break ;
#endif
				default:
					Error( "illegal flag" );
					}
		else
			namfil = *argv ;	// replaces "a.out"
		}

	if ( freopen( namfil , "r" , stdin ) == NULL )
		{
		perror( namfil );
		Error( "no exe file" );
		}

	if ( fread( &header , sizeof header , 1 , stdin ) != 1 )
		Error( "exe header unreadable" );
	switch ( header.a_magic )
		{
	case A_MAGIC1:
	case A_MAGIC2:
	case A_MAGIC3:
		break ;
	case A_MAGIC4:
		Error( "overlaid exe" );
	default:
		Error( "bad exe format" );
		}

	{
	long	symoffs = (long) header.a_text + (long) header.a_data ;

	if ( ! header.a_flag )		// relocation info present?
		symoffs *= 2 ;
	if ( fseek( stdin , symoffs , 1 ) == -1 )	// symtab start
		Error( "exe file too short" );
	}

	if ( (pf = fopen( monstr , "r" )) == NULL )
		Error( "no mon.out" );

	if ( fread( &lowpc , sizeof lowpc , 1 , pf ) != 1 ||
	     fread( &highpc , sizeof highpc , 1 , pf ) != 1 ||
	     fread( &ncount , sizeof ncount , 1 , pf ) != 1 )
		Error( "mon.out unreadable" );

	// The following is for 6th Edition UNIX (edit for 7th Edition):
	{
	long	bufsize ;		// monitor buffer size (bytes)

	if ( fstat( fileno( pf ) , &statbuf ) != 0 )
		{
		perror( monstr );
		Error( "can't happen" );
		}
	l3tol( &bufsize , statbuf.st_size , 1 );
	bufsize -= 3 * sizeof(unsigned) + ncount * sizeof cbuf[0] ;
	scale = sizeof(unsigned) *
		((highpc - lowpc) / (double)bufsize) ;
	}

	if ( ncount > MAXCALLS )
		Error( "too many calls" );

	if ( fread( cbuf , sizeof cbuf[0] , ncount , pf ) != ncount )
		Error( "mon.out unreadable" );

	// Read relevant symbol table entries into `nl' array:
	{
	unsigned	symsize = header.a_syms ;

	for ( np = &nl[0] , nname = 0 ; symsize > 0 ;
					symsize -= sizeof entry )
		{
		if ( fread( &entry , sizeof entry , 1 , stdin ) != 1 )
			Error( "exe truncated" );

		if ( (entry.n_type | aflg) == (N_EXT | N_TEXT) )
			{
			if ( ++nname > MAXNAMES )
				Error( "too many names" );

			strncpy( np->nl_name , entry.n_name , 8 );
			np->nl_value = entry.n_value ;
			np->nl_time = 0.0 ;
			np->nl_ncall = 0L ;

			++ np ;
			}
		}
	}

	np->nl_value = (unsigned)(-2) ;	// max possible value
	npe = np ;

	if ( nname == 0 )
		Error( "no symbols in exe file" );

	// Record procedure calls in name-list:
	{
	register struct cnt	*cp = &cbuf[0] , *cpe = &cbuf[ncount] ;

	for ( ; cp < cpe ; ++ cp )
		{
		unsigned	testval = cp->c_value - 8 ;

		for ( np = &nl[0] ; np < npe ; ++ np )
			if ( testval == np->nl_value )
				{
				np->nl_ncall = cp->c_ncall ;
				break ;
				}
		}
	}

	qsort( nl , nname , sizeof nl[0] , valcmp );	// sort by value

	// Assign times:
	{
	unsigned	ccnt ;

	totime = maxtime = 0.0 ;
	for ( bin = 0 ; fread( &ccnt , sizeof ccnt , 1 , pf ) == 1 ;
						++ bin )
		{
		register int	j ;	// indexes name-list
		unsigned	pcl , pch ;	// bin range
		double		time = (double)ccnt ;

		if ( ccnt == 0 )
			continue ;

		if ( time > maxtime )
			maxtime = time ;
		totime += time ;	// tally time

		// spread time uniformly over symbols in sample bin
		pcl = lowpc + scale * bin - 2.0 ;
		pch = lowpc + scale * (bin + 1) - 2.0 ;
		for ( j = 0 ; j < nname && nl[j].nl_value <= pch ; ++ j )
			if ( nl[j+1].nl_value > pcl )
				{
				int	overlap = (int)
					(min( pch , nl[j+1].nl_value ) -
					 max( pcl , nl[j].nl_value ));
				double	portion = (double)overlap * time
					/ scale ;

				nl[j].nl_time += portion ;
				}
		}

	if ( totime == 0.0 )
		Error( "no time accumulated" );
	}

#ifdef	PLOT
	if ( vflg )
		{
		char		buf[9] ;
		register int	j ;
		int		lowbin , highbin ;
		unsigned	ccnt ;
		double		bscale , lastsx , lastx , lasty , temp ;

		/* Call interface routines to generate plot */

		Vector( -2048.0 , -2048.0 , -2048.0 , 2048.0 );
		Vector( 0.0 , -2048.0 , 0.0 , 2048.0 );
		for ( j = 0 ; j < 9 ; ++ j )
			Vector( -2048.0 , 2048.0 - j * 512.0 ,
				0.0 , 2048.0 - j * 512.0 );

		lastsx = lastx = 0.0 ;
		lasty = 2048.0 ;
		temp = (highpc - lowpc) ;
		highpc = lowpc + temp * high / 100.0 ;
		lowpc = lowpc + temp * low / 100.0 ;
		scale = 4096.0 / (highpc - lowpc) ;
		bscale = scale * temp / bin ;
		lowbin = (double)bin * low / 100.0 ;
		highbin = (double)bin * high / 100.0 ;

		fseek( pf , (long)(ncount * sizeof cbuf[0] +
				   3 * sizeof(unsigned)) , 0 );

		for ( bin = 0 ;
		fread( &ccnt , sizeof ccnt , 1 , pf ) == 1 ; ++ bin )
			{
			double	nexty , time ;
			int	show = bin >= lowbin && bin < highbin ;

			// cumulative distribution function
			nexty = lasty - bscale ;
			time = (double)ccnt ;

			temp = lastsx ;
			lastsx -= 2000.0 * time / totime ;
			if ( show )
				Vector( temp , lasty , lastsx , nexty );

			if ( ccnt != 0 || lastx != 0.0 )
				{	// histogram
				temp = lastx ;
				lastx = - time * 2000.0 / maxtime ;
				if ( show )
					Vector( temp , lasty ,
						lastx , lasty );

				if ( show && ccnt != 0 )
					Vector( lastx , lasty ,
						lastx , nexty );
				}

			if ( show )
				lasty = nexty ;
			}

		// Put labels alongside:
		lastx = 50.0 ;
		for ( np = &nl[0] ; np < npe ; ++ np )
			{
			register char	*bp = buf ;

			if ( np->nl_value < lowpc ||
			     np->nl_value >= highpc )
				continue ;

			lasty = 2048.0 - (np->nl_value - lowpc) * scale ;
			Vector( 0.0 , lasty , 50.0 , lasty );
			Vector( lastx - 50.0 , lasty , lastx , lasty );

			for ( j = 0 ; j < 8 ; ++ j )
				if ( np->nl_name[j] != `_' )
					*bp++ = np->nl_name[j] ;
			*bp = `\0' ;
			Text( lastx + 10.0 , lasty - 60.0 , buf );
			lastx += 500.0 ;
			if ( lastx > 2000.0 )
				lastx = 50.0 ;
			}

		exit( 0 );
	}
#endif

	/* ASCII output to stdout */

	printf( "\s\s\s\sname %%time #call ms/call\n" );

	if ( ! lflg )
		qsort( nl , nname , 18 , timcmp );	// sort by time

	for ( np = &nl[0] ; np < npe ; ++ np )
		{
		double	time = 100.0 * np->nl_time / totime ;
		double	fnc = (double) np->nl_ncall ;

		printf( "%8.8s%6.1f" , np->nl_name , time );

		if ( np->nl_ncall > 0 )
			printf( "%6D %7.2f\n" , np->nl_ncall ,
				np->nl_time / (np->nl_ncall * 0.060) );
		else
			putchar( `\n' );
		}

	exit( 0 );
	}
/* Utility subroutines for "prof" */


static int
valcmp( p1 , p2 )			// for "qsort"
	struct nl	*p1 , *p2 ;
	{
	return ( (int)(p1->nl_value - p2->nl_value) );
	}


static int
timcmp( p1 , p2 )			// for "qsort"
	struct nl	*p1 , *p2 ;
	{
	float	d = p2->nl_time - p1->nl_time ;

	return ( d > 0.0 ? 1 : d < 0.0 ? -1 : 0 );
	}


static
Error( msg )				// fatal error notification
	char	*msg ;
	{
	fprintf( stderr , "\07* prof * %s\n" , msg );
	exit( 1 );
	}
/* TIGpack interface routines */

#ifdef	PLOT

extern	movepen(), pendown(), penup(), symbol();

#define	XSCALE	(6000.0 / 4096.0)
#define	YSCALE	(5000.0 / 4096.0)
#define	clip( c )	min( max( -2048.0 , c ) , 2047.0 )
#define	cvtx( xc )	((clip( xc ) + 2048.0) * XSCALE + 0.5)
#define	cvty( yc )	((clip( yc ) + 2048.0) * YSCALE + 0.5)

static
Vector( x0 , y0 , x1 , y1 )
	double	x0 , y0 , x1 , y1 ;
	{
	unsigned	xx0 = cvtx( x0 ) ,	// TIG UPA coords
			yy0 = cvty( y0 ) ,
			xx1 = cvtx( x1 ) ,
			yy1 = cvty( y1 ) ;

	penup();
	movepen( xx0 , yy0 );
	pendown();
	movepen( xx1 , yy1 );
	}


#define	CHARHT	(unsigned)(50.0 * XSCALE + 0.5)


static
Text( x , y , str )
	double	x , y ;
	char	*str ;
	{
	unsigned	xx = cvtx( x ) ,
			yy = cvty( y ) ;

	symbol( str , xx , yy , CHARHT , 0.0 );
	}
