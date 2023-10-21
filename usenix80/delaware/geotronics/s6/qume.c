#
/*
 * qume.c -	transmit input files to buffered Qume printer
 *
 * last edit: 10-Jun-1980  D A Gwyn
 *
 *	This filter synchronizes output to a printer using the ETX / ACK
 *	protocol.  It should be the last filter in a pipeline.  Do not
 *	direct its output to a file.
 */

#define	BUFFSIZE	224		/* chars in terminal's buffer */

#define	NO	0
#define	YES	1

#define	RONLY	0

#define	ACK	6
#define	ETX	3
#define	RAW	040
#define	CRMOD	020
#define	XTABS	02

struct	{
	char	ispeed, ospeed;
	char	erase, kill;
	int	mode;
}	*tp;

int	restore();

int	svmode;

main( argc, argv )
int	argc;
char	*argv[];
{
	register	arg, cc, first;
	int	c;

	if ( (signal( 2, 1 ) & 01) == 0 )
		signal( 2, &restore );	/* restore tty mode on break */
	save();				/* save tty mode */
	first = YES;			/* first_bufferload flag */
	cc = 0;				/* nothing in buffer yet */
	arg = 1;
	do  {
		if ( argc > 1 )  {
			close( 0 );
			open( argv[arg], RONLY );
		}
		while ( read( 0, &c, 1) == 1 )  {
			if ( ++cc >= BUFFSIZE/2 )  {
				if ( first )
					first = NO;	/* once-only */
				else  {
					putchar( ETX );
					holdup();
				}
				cc = 1;
			}
			putchar( c );
		}
	}  while ( ++arg < argc );
	restore();
}

holdup()
{
	char	c;

	tp->mode =| RAW;
	if ( stty( 2, tp ) < 0 )
		printf( "* first stty failed *" );
	else  {
		do  {
			while ( read( 2, &c, 1 ) < 1 )
				;
		}  while ( c != ACK );
		tp->mode =& ~RAW;
		if ( stty( 2, tp ) < 0 )
			printf( "* second stty failed *" );
	}
}

save()
{
	if ( gtty( 2, tp ) < 0 )  {
		printf( "* gtty failed *" );
		exit();
	}
	else  {
		svmode = tp->mode;
		tp->mode =& ~CRMOD;	/* turn off cr-lf */
		tp->mode =| XTABS;	/* turn off tabs */
		if ( stty( 2, tp ) < 0 )
			printf( "* turnoff failed *" );
	}
}

restore()
{
	tp->mode = svmode;
	if ( stty( 2, tp ) < 0 )
		printf( "* restore failed *" );
}
