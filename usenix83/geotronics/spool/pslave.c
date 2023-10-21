/*
	pslave -- H.I. CPS-15/6 pen-plotter device slave for despooler


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

19-Oct-1981	D A Gwyn	Created.
22-Oct-1981	D A Gwyn	UNIX raw mode parity handling changed.
24-Oct-1981	D A Gwyn	Now marks at starting edge of plot.
05-May-1982	D A Gwyn	Changed `\s' to standard syntax `\ '.


Usage:
	Invoked by despooler to transfer file to plotter, as
	% pslave file device
	Note: device MUST be a tty port.

Method:
	Plot data is chopped into 512-byte blocks (or less, for final
	block) and wrapped with protocol codes before being sent to
	the tty port.  In the default handshaking mode, the returned
	status is not examined until the next block has been readied
	for transmission, to provide additional throughput by double-
	buffering.  Retries are performed on the previous block if
	necessary, then the already-prepared next block is sent.

	To use XON/XOFF control of data flow, #define "XON_XOFF".
	In this mode no transmission status feedback is possible.

	The plotter MUST be jumpered (W35) to ignore parity.
*/

#include	<sgtty.h>
#include	<signal.h>

/*#define	XON_XOFF	/* for XON/XOFF data flow control */

#ifndef BAUD
#define BAUD	B9600			/* baud rate */
#endif

#ifndef MAXTRY
#define MAXTRY	2			/* max transmit attempts */
#endif

#define MAXBLK	512			/* max bytes per block */
/* If you change the following, update buf0.data.block[] also. */
#define PROMPT	'\r'			/* prompt/inquire code */

struct	buf
	{
	struct buf	*next;		/* -> next buffer */
	int		count;		/* # bytes transmitted */
	struct	{
		char	leadin[2];	/* standard H.I. preamble */
		char	bcount[2];	/* sixbit byte count */
		char	block[MAXBLK+1];/* data bytes plus PROMPT */
		}	data;		/* transmitted record */
	};

static struct buf	buf0 =		/* initially, setup codes;   */
	{				/*   then second data buffer */
	0,				/* initialized to &buf1 */
	56,				/* 51+5 characters */
		{
		'\n', '#',
		' ', 'S',		/* 51 characters */
		/* The following setup string selects pen 1 and makes
		   0.5" marks near both edges to aid in cutting plots.
		   String length of 51 is "known" as indicated above. */
#ifdef XON_XOFF
	     "X40DX501X800PR1B )/ \ )/ \ >-7Q ?8&P 7. \ 7. A %<Q ':P\r",
#else
	     "X40DX500X800PR1B )/ \ )/ \ >-7Q ?8&P 7. \ 7. A %<Q ':P\r",
#endif
		}
	};

static struct buf      buf1		/* first data buffer */
	{
	&buf0,				/* -> other data buffer */
	-1,
		{
		'\n', '#',
		-1, -1,
		"data goes here\r",
		}
	};

#ifndef XON_XOFF
static char		stat[3];	/* status from handshake */
#endif
static int		ifd;		/* input file */
static int		ofd;		/* plot device */
static int		retries;	/* retransmission counter */
static int		ir;		/* # bytes read */
static struct sgttyb	tty;		/* for "stty" */


main( argc, argv )
	int	argc;
	char	*argv[];
	{
	register struct buf	*curbuf = &buf0;/* transmitted */
	register struct buf	*nextbuf;	/* being prepared */

	buf0.next = &buf1;		/* can't be done statically */
	if ( argc != 3 )
		_exit( 1 );		/* Usage: pslave file device */

	/* Initialization: open files; set mode; send setup codes. */

	signal( SIGHUP, SIG_DFL );	/* exit on plotter off-line */

	if ( (ifd = open( argv[1], 0 )) < 0 )
		_exit( 2 );		/* no input file */

	if ( (ofd = open( argv[2], 2 )) < 0 )
		_exit( 3 );		/* no output device */

	if ( gtty( ofd, &tty ) < 0 )
		_exit( 4 );		/* not a tty */
	tty.sg_flags = 0;
	stty( ofd, &tty );		/* flush input queue */
	tty.sg_ispeed = tty.sg_ospeed = BAUD;
	tty.sg_erase = 010;		/* not used */
	tty.sg_kill = 025;		/* not used */
	tty.sg_flags = RAW;		/* 8 bits wide on newer UNIX */
	stty( ofd, &tty );		/* set up parameters */

	if ( write( ofd, ";#", 2 ) != 2 )	/* device select */
		_exit( 5 );		/* write failed */
	/* Note that main loop will transmit setup codes. */

	/* Main loop: write current buffer; fill next buffer;
	     check current status (retry?); send next buffer. */

	do	{
		/* The following write would be better asynchronous. */
		if ( write( ofd, &curbuf->data, curbuf->count )
		     != curbuf->count
		   )
			_exit( 6 );	/* write failed */

		nextbuf = curbuf->next;

		if ( (nextbuf->count =
		      read( ifd, nextbuf->data.block, MAXBLK )
		     ) < 0
		   )
			_exit( 7 );	/* read failed */

		if ( nextbuf->count )	/* not EOF */
			{
			nextbuf->data.block[nextbuf->count] = PROMPT;

			nextbuf->data.bcount[0] =
				(nextbuf->count >> 6) + ' ';
			nextbuf->data.bcount[1] =
				(nextbuf->count & 077) + ' ';

			nextbuf->count += 5;	/* allow for controls */
			}
#ifndef XON_XOFF
		/* Wait for previous transmission to succeed. */

		for ( retries = 0; ; )
			{
			register int	nr;	/* total of `ir's */

			for ( nr = 0; nr < 3; nr += ir )
				{	/* minimize system calls */
				if ( (ir = read( ofd, &stat[nr], 3-nr ))
								<= 0 )
					_exit( 8 );	/* read fail */
				}

			if ( (stat[2] & 0177) != '\r' )
				_exit( 9 );	/* garbled */

			if ( (stat[0] & 0177) == '0' )
				break;	/* successfully received */

			if ( ++retries >= MAXTRY )
				_exit( 10 );	/* too many errors */

			/* Retransmit previous block. */

			if ( write( ofd, &curbuf->data, curbuf->count )
			     != curbuf->count
			   )
				_exit( 11 );	/* write failed */
			}
#endif
		curbuf = nextbuf;	/* flip-flop buffers */
		} while ( nextbuf->count );	/* not EOF */

	/* Deactivate plotter. */

	if ( write( ofd, "\n# \ ", 4 ) != 4 )	/* device deselect */
		_exit( 12 );		/* write failed */

	_exit( 0 );			/* successful */
	}
