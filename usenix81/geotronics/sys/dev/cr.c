/*
	cr -- CR11/CM11 card reader driver for 6th Edition UNIX

	last edit: 21-Mar-1981	D A Gwyn

Supports only one card reader - that should be enough for anybody!
Maps punch codes to ASCII; assumes 026 keypunch if odd minor device number.
Trailing spaces are removed and a newline is appended to each card image.
Hopper empty (or stacker full) is interpreted as end of file.
*/

#include	<sys/types.h>
#include	"../h/param.h"
#include	"../h/buf.h"
#include	"../h/conf.h"
#include	"../h/user.h"

#define	CRPRI	10		/* sleep priority */

/*	card reader controller registers (start at CRADDR)	*/

#define	CRADDR	0177160		/* CSR address (DEC standard 0177160) */

struct	{
	int	crs ;		// control & status register
#define		ERR	(1<<15)
#define		CDONE	(1<<14)
#define		HOPPER	(1<<13)
#define		MOTION	(1<<12)
#define		TIMING	(1<<11)
#define		ONLINET	(1<<10)
#define		BUSY	(1<<9)
#define		OFFLINE	(1<<8)
#define		COLRDY	(1<<7)
#define		INTENB	(1<<6)
#define		EJECT	(1<<1)
#define		READ	(1<<0)
	int	crb1 ;		// data buffer register (12 bits)
	int	crb2 ;		// data buffer register (compressed)
	};

static struct
	{
	char	state ;		// flags for handler:
#define		CLOSED	0	/* device available */
#define		OPEN	1	/* device in use */
#define		MAP026	2	/* use 026 keypunch mapping */
#define		ERROR	4	/* read error occurred */
#define		ENDFILE	8	/* hopper empty */
	int	nchars ;	// chars in buffer
	char	*bufp ;		// next loc. in buffer
	char	buffer[82] ;	// card image + NEWLINE + terminator
	}		cr11 ;	// driver status information

/*
	standard (029) punch code to ASCII translation table
	indexed by compressed 8-bit char (crb2)
*/
static char crcode[256] =
	{
	` ',`1',`2',`3',`4',`5',`6',`7',	//
	`8',``',`:',`#',`@',`\'',`=',`"',	//	     8
	`9', 0 , 22, 0 , 0 , 0 , 0 ,  4,	//	   9
	 0 , 0 , 0 , 0 , 20, 21 , 0 , 26,	//	   9 8
	`0',`/',`S',`T',`U',`V',`W',`X',	//       0
	`Y', 0 ,`\\',`,',`%',`_',`>',`?',	//       0   8
	`Z', 0 , 0 , 0 , 0 , 10, 23, 27,	//       0 9
	 0 , 0 , 0 , 0 , 0 ,  5,  6,  7,	//       0 9 8
	`-',`J',`K',`L',`M',`N',`O',`P',	//    11
	`Q', 0 ,`]',`$',`*',`)',`;',`^',	//    11     8
	`R', 17, 18, 19, 0 , 0 ,  8, 0 ,	//    11   9
	 24, 25, 0 , 0 , 28, 29, 30, 31,	//    11   9 8
	`}',`~',`s',`t',`t',`u',`w',`x',	//    11 0
	`y', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	//    11 0   8
	`z', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	//    11 0 9
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	//    11 0 9 8
	`&',`A',`B',`C',`D',`E',`F',`G',	// 12
	`H', 0 ,`[',`.',`<',`(',`+',`!',	// 12        8
	`I',  1,  2,  3, 0 ,  9, 0 ,127,	// 12      9
	 0 , 0 , 0 , 11, 12, 13, 14, 15,	// 12      9 8
	`{',`a',`b',`c',`d',`e',`f',`g',	// 12    0
	`h', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	// 12    0   8
	`i', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	// 12    0 9
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	// 12    0 9 8
	`|',`j',`k',`l',`m',`n',`o',`p',	// 12 11
	`q', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	// 12 11     8
	`r', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	// 12 11   9
	 0 , 16, 0 , 0 , 0 , 0 , 0 , 0 ,	// 12 11   9 8
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	// 12 11 0
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	// 12 11 0   8
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	// 12 11 0 9
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	// 12 11 0 9 8
	};


/*
	cropen -- open device for reading
*/

cropen( dev , wflag )
	dev_t	dev ;
	int	wflag ;
	{
	if ( cr11.state != CLOSED )
		u.u_error = EACCES ;
	else
		cr11.state = dev.d_minor & 01 ? OPEN | MAP026 : OPEN ;
	}


/*
	crclose -- close device
*/

crclose( dev , flag )
	dev_t	dev ;
	int	flag ;
	{
	CRADDR->crs = 0 ;		// disable interrupts
	cr11.state = CLOSED ;
	}


/*
	crread -- read data from device
*/

crread( dev )
	dev_t	dev ;
	{
	extern int	lbolt ;		// waken every 4 seconds
	register char	*bp , c ;
	register int	map ;

	if ( cr11.state & ENDFILE )
		return ;		// previous card was last

	cr11.state &= ~ ERR ;		// assume user took care of it
	cr11.bufp = &cr11.buffer[0] ;

	while ( CRADDR->crs & OFFLINE )
		sleep( &lbolt , CRPRI );	// wait for online

	spl6();				// avoid wakeup before sleep
	CRADDR->crs = INTENB | READ ;	// initiate card motion
	sleep( &cr11 , CRPRI );		// wait for completion	
	spl0();

	while ( CRADDR->crs & BUSY )	// wakeup due to ERR
		sleep( &lbolt , CRPRI );	// wait for card to clear

	bp = cr11.bufp ;		// normally &cr11.buffer[80]
	if ( cr11.state & ENDFILE && bp == &cr11.buffer[0] )
		return ;		// hopper empty or stacker full

	if ( bp > &cr11.buffer[u.u_count] )
		bp = &cr11.buffer[u.u_count] ;	// no more than requested
	while ( bp > &cr11.buffer[0] )	// trim trailing spaces
		if ( *--bp != ` ' )
			{
			++ bp ;		// one too far
			break ;
			}
	*bp++ = `\n' ;			// add newline
	*bp = -1 ;			// flag end of card image

	map = cr11.state & MAP026 ;
	for ( bp = &cr11.buffer[0] ; (c = *bp) >= 0 ; ++ bp )
		{
		if ( map )		// assume 026 keypunch used
			switch ( c )
				{
			case `#':
				c = `=' ;
				break ;
			case `@':
				c = `\'' ;
				break ;
			case `%':
				c = `(' ;
				break ;
			case `&':
				c = `+' ;
				break ;
			case `<':
				c = `)' ;
				break ;
			case 26:
				c = `~' ;	// special for 789 punch
				break ;
			// others unchanged
				}
		passc( c );		// pass chars to user
		}

	if ( cr11.state & ERR )
		u.u_error = EIO ;	// read failed
	}


/*
	crintr -- interrupt handler (runs at priority 6)

There are three causes of interrupts:
	character available in buffer (COLRDY);
	card done, ready for new read request (CDONE);
	motion, timing, or hopper/stacker error (ERR).
*/

crintr( subtype )
	int	subtype ;
	{
	register int	status = CRADDR->crs ;	// get status

	if ( status & COLRDY && cr11.bufp < &cr11.buffer[80] )	// safety
		*cr11.bufp++ = crcode[CRADDR->crb2] ;	// get ASCII

	if ( status < 0 )		// test for ERR
		if ( status & (MOTION | TIMING) )
			cr11.state |= ERROR ;	// read failed
		else			// hopper empty or stacker full
			cr11.state |= ENDFILE ;

	if ( status & (CDONE | ERR) )
		{
		CRADDR->crs = 0 ;	// disable interrupts
		wakeup( &cr11 );	// transfer complete
		}
	}
