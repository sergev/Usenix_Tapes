#
/*
 * pg.c -	paginate input files (can be used as filter)
 *
 * last edit: 10-Jun-1980  D A Gwyn
 *
 *	Copies argument files (or standard input, if no arguments) to
 *	standard output one screen-load at a time.  After each page,
 *	"More?" is asked; any response except "n" or EOT will present
 *	the next page.  "Next file?" is asked before each file except
 *	the first.
 */

#define	LINESIZE	80	/* characters before wrap-around */
#define	PAGESIZE	24	/* number of lines on terminal */

#define	NO	0
#define	YES	1

#define	RONLY	0

char	cbuf	0;

main( argc, argv )
int	argc;
char	*argv[];
{
	register	c, col, line;
	int	arg;

	arg = 1;
	do  {
		line = 1;
		c = col = 0;
		if ( argc > 1 )  {
			close( 0 );	/* only open first time */
			printf( "========== %s ==========\n",
							argv[arg] );
			line++;
			open( argv[arg], RONLY );
		}
		while ( read( 0, &cbuf, 1 ) == 1 )  {
			if ( line >= PAGESIZE )  {
				if ( ! More( NO ) )
					break;
				line = 1;
			}
			c = cbuf & 0177;
			switch ( c )  {
				case 014:	/* form-feed */
					line = PAGESIZE;
					c = '\n';
				case '\n':
					line++;
				case '\r':
					col = 0;
					break;
				case '\b':
					if ( col > 0 )
						col--;
					break;
				case '\t':
					col = col + 8  &  ~7;
					break;
				default:
					col++;
			}
			if ( col < LINESIZE )
				putchar( c );
		}
		if ( c != '\n' )
			putchar( '\n' );
		close( 0 );
	}  while ( ++arg < argc  &&  More( YES ) );
}

More( file )
int	file;
{
	register	a;
	char	c;

	if ( file )
		printf( "Next file?" );
	else
		printf( "More?" );
	for ( a = 0;  read( 2, &c, 1 ); )  {
		if ( a == 0 )
			a = c;
		if ( c == '\n' )
			return ( a != 'n' );
	}
	return ( NO );
}
