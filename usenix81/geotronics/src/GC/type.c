/*
 * type.c - touch-typing trainer
 *
 *	last edit: 21-Dec-1980	D A Gwyn
 *
 * If invoked without arguments, suppresses "beep" on each character.
 */

#include	<sgtty.h>
#include	<signal.h>
#include	<stdio.h>

static char	alpha[] =
	{
		'A','A','A','A','A','A','A','A',
		'B','B',
		'C','C','C',
		'D','D','D','D',
		'E','E','E','E','E','E','E','E','E','E','E','E','E',
		'F','F','F',
		'G','G',
		'H','H','H','H','H',
		'I','I','I','I','I','I',
		'J',
		'K',
		'L','L','L','L',
		'M','M','M',
		'N','N','N','N','N','N',
		'O','O','O','O','O','O',
		'P','P',
		'Q',
		'R','R','R','R','R','R',
		'S','S','S','S','S','S',
		'T','T','T','T','T','T','T','T','T','T',
		'U','U','U',
		'V','V',
		'W','W',
		'X',
		'Y','Y',
		'Z',
		'.','.',
		',',',',',',
		' ',' ',' ',' ',' ',' ',' ',
		'-',
		';',';',
		':',
		'(',
		')',
		'!',
		'?',
		'\'','\'',
		'"',
	};
#define	EOT	04

static			onintr();
static struct sgttyb	tty ;
static int		mode ;


main( argc , argv )
	int		argc ;
	char		*argv[];
	{
	char		c ;
	register int	next ;		/* index alpha */

	{
	int	timeval[2];
	time( timeval );
	srand( timeval[1] );		/* random seed */
	}
	gtty( 0 , &tty );
	mode = tty.sg_flags ;		/* save for exit */
	tty.sg_flags &= ~(CRMOD|ECHO|LCASE|XTABS);
	tty.sg_flags |= RAW ;
	signal( SIGTERM , onintr );
	stty( 0 , &tty );

	printf( "\n-> <-\b\b\b" );

	for ( ; ; )
		{
		next = rand() % sizeof alpha ;
		do	{
			if ( argc >= 2 )
				putchar( '\07' );
			putchar( alpha[next] );
			putchar( '\b' );
			if ( (c = getchar()) == EOT )
				onintr();	/* exit */
			if ( c >= 'a' & c <= 'z' )
				c -= 'a' - 'A';	/* make upper case */
			if ( c != alpha[next] )
				putchar( '\07' );
			} while ( c != alpha[next] );
		}
	}


static
onintr()				/* termination */
	{
	tty.sg_flags = mode ;
	stty( 0 , &tty );
	putchar( '\n' );
	exit( 0 );
	}
