/*
	bslave -- simple-minded device slave for band printer


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

25-Apr-1982	D A Gwyn	Created.
12-May-1982	D A Gwyn	Improved signal handling.


Usage:
	Invoked by despooler to transfer file to output device, as
	% bslave file device
	NOTE: device MUST be a tty port.

Method:
	Just a straight file copy with interpretation of tabs,
	simplistic line overstrike handling (for backspace, etc.),
	and mapping of random control characters to printable form.
*/

#include	<sgtty.h>
#include	<signal.h>
#include	<stdio.h>

extern char	_sobuf[BUFSIZ];

static	onintr();

/*#define	IGN_NULL		/* to ignore NUL characters */

#ifndef BAUD
#define BAUD	B9600			/* baud rate */
#endif

static struct sgttyb	tty;		/* for "stty" */

main( argc, argv )
	int	argc;
	char	*argv[];
	{
	register int	c;		/* input character */
	register int	icol;		/* input column counter */
	register int	ocol;		/* output column counter */

	if ( argc != 3 )
		_exit( 1 );		/* Usage: bslave file device */

	/* signals come from controlling tty, not from printer port */
	signal( SIGHUP, SIG_IGN );
	signal( SIGINT, SIG_IGN );
	signal( SIGQUIT, SIG_IGN );
	signal( SIGTERM, onintr );

	if ( freopen( argv[1], "r", stdin ) == NULL )
		_exit( 2 );		/* no input file */

	if ( freopen( argv[2], "w", stdout ) == NULL )
		_exit( 3 );		/* no output device */
	setbuf( stdout, _sobuf );	/* force buffered output */

	if ( gtty( fileno( stdout), &tty ) < 0 )
		_exit( 4 );		/* not a tty port */
	tty.sg_flags = 0;		/* (cooked input) */
	stty( fileno( stdout ), &tty );
	tty.sg_ispeed = tty.sg_ospeed = BAUD;
	tty.sg_erase = 010;		/* not used */
	tty.sg_kill = 025;		/* not used */
	tty.sg_flags = RAW;		/* flush input if any */
	stty( fileno( stdout ), &tty ); /* set up parameters */

	putchar( '\f' );		/* initial form-feed */
	icol = ocol = 0;

	while ( (c = getchar()) != EOF )
		switch ( c &= 0177 )	/* strip parity if any */
			{
#ifdef	IGN_NULL
		    case 0:		/* NUL */
			break;
#endif
		    case '\b':		/* BS */
			if ( icol > 0 )
				--icol;
			break;
		    case '\r':		/* CR */
		    case '\n':		/* LF */
		    case '\f':		/* FF */
			putchar( c );
			icol = ocol = 0;
			break;
		    case ' ':		/* SP */
			++icol;
			break;
		    case '\t':		/* HT */
			do		/* DEC standard tab stops */
				++icol;
				while ( icol % 8 != 0 );
			/* This could be sped up, but not by much. */
			break;
		    default:
			if ( c == 0177 )	/* DEL is dangerous */
				{	/* map to "^/" */
				ungetc( '/', stdin );
				c = '^';
				}
			else if ( c < 040 )	/* control character */
				{	/* map to printing equivalent */
				ungetc( c + 0100, stdin );
				c = '^';	/* e.g., "^X" */
				}
			/* at this point, c is printable */
			if ( icol < ocol )
				{	/* need overprint line */
				putchar( '\r' );
				ocol = 0;
				}
			while ( ocol < icol )
				{	/* catch up to virtual pos */
				putchar( ' ' );
				++ocol;
				}
			putchar( c );
			icol = ++ocol;
			}			/* end switch */

	if ( icol != 0 || ocol != 0 )
		putchar( '\n' );	/* force print cycle */

	fflush( stdout );
	stty( fileno( stdout ), &tty ); /* (wait for output to drain) */

	if ( ferror( stdin ) )
		exit( 4 );		/* read failed */
	if ( ferror( stdout ) )
		exit( 5 );		/* write failed */

	exit( 0 );			/* successful */
	}

static
onintr( sig )				/* called on SIGTERM */
	int	sig;			/* not used */
	{
	signal( SIGTERM, SIG_IGN );	/* just in case */

	write( fileno( stdout ), "\n", 1 );	/* force print cycle */

	_exit( 6 );			/* terminated */
	}
