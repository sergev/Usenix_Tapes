/*
	mesg -- set terminal access mode (compatible with 7th Ed. UNIX)

	last edit: 06-Jan-1981	D A Gwyn

Usage:
	% mesg n	forbid messages
	% mesg y	allow messages
	% mesg		report current state
*/

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>

extern char		*ttyname();
extern int		stat();

static struct stat	buf ;

main( argc , argv )
	int	argc ;
	char	*argv[];
	{
	register char	*tty ;
	register int	f ;

	for ( f = 2 ; f >= 0 ; -- f )
		if ( (tty = ttyname( f )) != NULL )
			break ;
	if ( tty == NULL )
		error( "Can't locate terminal" );

	if ( stat( tty , &buf ) != 0 )
		error ( "stat failed" );

	if ( --argc > 0 )		/* have an argument */
		{
		register int	mode ;

		switch ( (*++argv)[0] )
			{
		case 'n':
			mode = 0600 ;
			buf.st_mode &= ~(S_IWRITE>>6) ;
			break ;
		case 'y':
			mode = 0622 ;
			buf.st_mode |= S_IWRITE>>6 ;
			break ;
		default:
			error( "Bad argument" );
			}
		if ( chmod( tty , mode ) != 0 )
			error( "Can't change mode" );
		}

	f = 1 - ((buf.st_mode & S_IWRITE>>6)>>1);

	if ( argc <= 0 )		/* no arguments */
		fputs( f ? "is n\n" : "is y\n" , stderr );

	exit( f );
	}


static
error( msg )				/* print fatal error message */
	char	*msg ;
	{
	fputs( "\07* mesg * " , stderr );	/* avoid "fprintf" */
	fputs( msg , stderr );
	putc( '\n' , stderr );
	exit( 2 );
	}
