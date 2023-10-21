/*
	ln -- link to file (compatible with 7th Edition UNIX)

	last edit: 13-Apr-1981	D A Gwyn
*/

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>

extern			perror();
extern char		*rindex();
extern int		link() , stat();

static			error();
static struct stat	statb ;


main( argc , argv )
	register int	argc ;
	char		*argv[];
	{
	register char	*targ = argv[1] , *newn ;

	if ( --argc < 1 || argc > 2 )
		error( "Usage: ln target [ newname ]" );

	if ( argc == 1 )
		if ( (newn = rindex( targ , '/' )) == NULL )
			newn = targ ;	/* no `/' in target name */
		else
			++ newn ;	/* past last `/' in name */
	else
		newn = argv[2];

	if ( stat( targ , &statb ) != 0 )
		error( "target non-existent" );
	if ( (statb.st_mode & S_IFMT) == S_IFDIR )
		error( "can't link to directory" );
	if ( link( targ , newn ) != 0 )
		{
		perror( newn );
		error( "link failed" );
		}

	exit( 0 );
	}


static
error( msg )				/* print fatal error message */
	char	*msg ;
	{
	fputs( "\07* ln * " , stderr );	/* avoid "fprintf" */
	fputs( msg , stderr );
	putc( '\n' , stderr );
	exit( 1 );
	}
