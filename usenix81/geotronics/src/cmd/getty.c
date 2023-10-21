/*
	getty -- adapt to terminal speed on dialup, and call login

	last edit:	22-Jun-1981	D A Gwyn

1) fixed bug in UC/lc detection;
2) changed terminal-type table;
3) erase, kill chars changed to BS, NAK.
*/

#include	<sgtty.h>
#include	<stdio.h>

#define	ERASE	'\b'
#define	KILL	025

struct sgttyb	tmode ;

struct	tab
	{
	char	tname ;			// this table name
	char	nname ;			// successor table name
	int	iflags ;		// initial flags
	int	fflags ;		// final flags
	char	ispeed ;		// input speed
	char	ospeed ;		// output speed
	char	*message ;		// login message
	}	itab[] =
	{

	/* table '-' - operator console */

	'-' , '-' ,
	ANYP + RAW , ANYP + XTABS + ECHO + CRMOD ,
	B9600 , B9600 ,
	"\r\nlogin: " ,

	/* table '0'-1-2 1200,300,110 dialups */

	'0' , 1 ,
	ANYP + RAW , ANYP + XTABS + ECHO + CRMOD ,
	B1200 , B1200 ,
	"\r\nlogin: " ,

	1 , 2 ,
	ANYP + RAW , ANYP + XTABS + ECHO + CRMOD ,
	B300 , B300 ,
	"\r\nlogin: " ,

	2 , '0' ,
	ANYP + RAW + NL1 + CR1 ,
			ANYP + XTABS + ECHO + CRMOD + LCASE + CR1 ,
	B110 , B110 ,
	"\r\nlogin: " ,

	/* table '1'-3-4-5 9600,1200,300 DEC terminals & 110 TTY */

	'1' , 3 ,
	ANYP + RAW , ANYP + XTABS + ECHO + CRMOD ,
	B9600 , B9600 ,
	"\r\nlogin: " ,

	3 , 4 ,
	ANYP + RAW , ANYP + XTABS + ECHO + CRMOD ,
	B1200 , B1200 ,
	"\r\nlogin: " ,

	4 , 5 ,
	ANYP + RAW , ANYP + XTABS + ECHO + CRMOD ,
	B300 , B300 ,
	"\r\nlogin: " ,

	5 , '1' ,
	ANYP + RAW + NL1 + CR1 ,
			ANYP + XTABS + ECHO + CRMOD + LCASE + CR1 ,
	B110 , B110 ,
	"\r\nlogin: " ,

	/* table '2'-6 1200,300 in-house hardcopy terminals */

	'2' , 6 ,
	ANYP + RAW , ANYP + XTABS + ECHO + CRMOD ,
	B1200 , B1200 ,
	"\r\nlogin: " ,

	6 , '2' ,
	ANYP + RAW , ANYP + XTABS + ECHO + CRMOD ,
	B300 , B300 ,
	"\r\nlogin: " ,

	};

#define	NITAB	(sizeof itab / sizeof itab[0])

static char	name[16] ;
static char	crmod ;
static char	upper ;
static char	lower ;
static int	getname();


main( argc , argv )
	int	argc ;
	char	*argv[] ;
	{
	register struct tab	*tabp ;
	register char		tname ;

	if ( argc > 1 )
		tname = *argv[1] ;
	else
		tname = `-' ;		// default

	for ( ; ; )
		{
		for ( tabp = itab ; tabp < &itab[NITAB] ; ++ tabp )
			if ( tabp->tname == tname )
				break ;
		if ( tabp >= &itab[NITAB] )
			tabp = itab ;

		tmode.sg_ispeed = tabp->ispeed ;
		tmode.sg_ospeed = tabp->ospeed ;
		tmode.sg_erase = ERASE ;
		tmode.sg_kill = KILL ;
		tmode.sg_flags = tabp->iflags ;
		stty( fileno( stdin ) , &tmode );

		fputs( tabp->message , stdout );
		stty( fileno( stdin ) , &tmode );

		if ( getname() )
			{
			tmode.sg_flags = tabp->fflags ;
			if ( crmod )
				tmode.sg_flags |= CRMOD ;
			if ( upper )
				tmode.sg_flags |= LCASE ;
			if ( lower )
				tmode.sg_flags &= ~ LCASE ;
			stty( fileno( stdin ) , &tmode );

			execl( "/bin/login" , "login" , name , NULL );
			exit( 1 );
			}
		tname = tabp->nname ;
		}
	}

static int
getname()
	{
	register char	*np = name ;
	register int	c ;
	static int	cs ;

	crmod = upper = lower = 0 ;

	do	{
		if ( (cs = getchar()) == EOF )
			exit( 0 );
		if ( (c = cs & 0177) == 0 )
			return ( 0 ) ;	// break
		if ( c != `\b' || np > name )
			putchar( cs );
		if ( c >= `a' && c <= `z' )
			++ lower ;
		else if ( c >= `A' && c <= `Z' )
			{
			++ upper ;
			c += `a' - `A' ;
			}
		else if ( c == ERASE )
			{
			if ( np > name )
				{
#if	ERASE == '\b'
				fputs( " \b" , stdout );
#endif
				c = *--np ;
				if ( c >= `a' && c <= `z' )
					-- lower ;
				else if ( c >= `A' && c <= `Z' )
					-- upper ;
				}
			continue ;
			}
		else if ( c == KILL )
			{
#if	ERASE == '\b'
			for ( ; np > name ; -- np )
				fputs( "\b \b" , stdout );
#else
			fputs( "^U\n" , stdout );
			np = name ;
#endif
			upper = lower = 0 ;
			continue ;
			}
		*np++ = c ;
		} while ( c != `\n' && c != `\r' && np <= &name[16] ) ;
	*--np = 0 ;
	if ( c == `\r' )
		{
		putchar( `\n' );
		++ crmod ;
		}
	else
		putchar( `\r' );
	return ( 1 ) ;
	}
