/*
 * pg - paginate input files (can be used as filter)
 *
 *	last edit: 21-Dec-1980	D A Gwyn
 *
 *	Copies argument files (or standard input, if no arguments) to
 *	standard output one screen-load at a time.  After each page,
 *	"More?" is asked; any response except "n" or EOT will present
 *	the next page.  "Next file?" is asked before each file except
 *	the first.  Interrupt and quit abort "pg".
 */

#include	<sgtty.h>
#include	<signal.h>
#include	<stdio.h>

#define	LINESIZE	80	/* characters before wrap-around */
#define	PAGESIZE	24	/* number of lines on terminal */

#define	NO	0
#define	YES	1

static char		obuf[BUFSIZ];
static struct sgttyb	tty ;
static int		mode ;

static	onintr(), more();


main( argc , argv )
	int		argc;
	char		*argv[];
	{
	register int	c , col , line ;
	int		arg , lastc ;

	if ( ! isatty( fileno( stdout ) ) )
		{
		fprintf( stderr , "You turkey!\n" );
		exit( 1 );
		}
	setbuf( stdout , obuf );

	if ( freopen( "/dev/tty" , "r" , stderr ) == NULL )
		exit( 2 );
	gtty( fileno( stderr ) , &tty );
	mode = tty.sg_flags ;		/* save for exit */

	arg = 1 ;
	do	{
		line = 1 ;
		col = 0 ;
		lastc = '\n';
		if ( argc > 1 )
			{
			if ( freopen( argv[arg] , "r" , stdin ) == NULL )
				printf( "***** %s: can't open *****\n" ,
				argv[arg] );
			else
				printf( "========== %s ==========\n" ,
				argv[arg] );
			++ line ;
			}
		while ( (c = getchar()) != EOF )
			{
			if ( line >= PAGESIZE )
				{
				if ( ! more( NO ) )
					break ;
				line = 1 ;
				}
			switch ( c )
				{
			case '\v':
			case '\f':
				line = PAGESIZE ;
				c = '\n' ;
			case '\n':
				++ line ;
			case '\r':
				col = 0 ;
				break ;
			case '\b':
				if ( col > 0 )
					-- col ;
				break ;
			case '\t':
				col = col + 8 & ~7 ;
				break ;
			default:
				if ( c < ' ' )	/* map ctrl char */
					{
					ungetc( c + '@' , stdin );
					c = '^';
					}
				++ col ;
				}
			if ( col < LINESIZE )
				putchar( lastc = c );
			}
		if ( lastc != '\n' )
			putchar( '\n' );
		} while ( ++arg < argc && more( YES ) );
	exit( 0 );
	}


#define	CEOT	0004			/* end-of-file */
#define	CINT	0177			/* interrupt */
#define	CQUIT	0034			/* quit */


static
more( file )
	register int	file ;
	{
	register int	c ;

	printf( file ? "Next file? " : "More? " );
	fflush( stdout );

	tty.sg_flags &= ~ECHO ;
	tty.sg_flags |= RAW ;
	signal( SIGTERM , onintr );
	stty( fileno( stderr ) , &tty );
	c = getc( stderr ) & 0177 ;
	tty.sg_flags = mode ;
	stty( fileno( stderr ) , &tty );
	signal( SIGTERM , SIG_DFL );
	putchar( '\n' );

	if ( c == CINT || c == CQUIT )
		exit( 3 );

	return ( c != CEOT && c != 'n' && c != 'N' );
	}


static
onintr()
	{
	tty.sg_flags = mode ;
	stty( fileno( stderr ) , &tty );
	exit( 3 );
	}
