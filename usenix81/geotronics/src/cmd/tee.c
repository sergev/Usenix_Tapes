/*
	tee -- split pipe into files (compatible with 7th Edition UNIX)

	last edit: 06-Jan-1981	D A Gwyn
*/

#include	<signal.h>
#include	<stdio.h>

#define	MAXFILES	13
static FILE	*op[MAXFILES] = { stdout , NULL };
static char	*name[MAXFILES] = { "stdout" , NULL };
static int	nfiles = 1 ;		/* includes stdout */

static char	*mode = "w" ;		/* -a flag changes this to "a" */


main( argc , argv )
	int	argc ;
	char	*argv[];
	{
	register int	c , file ;

	while ( --argc > 0 )
		if ( (*++argv)[0] == '-' )	/* it's a flag */
			switch ( (*argv)[1] )
				{
			case 'a':
				mode = "a" ;
				break ;
			case 'i':
				signal( SIGHUP , SIG_IGN );
				signal( SIGINT , SIG_IGN );
				signal( SIGQUIT , SIG_IGN );
				break ;
			default:
				error( "Usage: .. | tee[ -ai][ files] | .." );
				}
		else	{			/* it's a filename */
			if ( nfiles >= MAXFILES )
				error ( "too many files" );
			name[nfiles] = *argv ;
			if ( (op[nfiles++] = fopen( *argv , mode )) == NULL )
				{
				perror( *argv );
				error( "can't create file" );
				}
			}
	while ( (c = getchar()) != EOF )
		for ( file = 0 ; file < nfiles ; ++ file )
			{
			putc( c , op[file] );
			if ( ferror( op[file] ) )
				{
				perror( name[file] );
				error( "write error" );
				}
			}
	exit( 0 );
	}


static
error( msg )				/* print fatal error message */
	char	*msg ;
	{
	fputs( "\07* tee * " , stderr );	/* avoid "fprintf" */
	fputs( msg , stderr );
	putc( '\n' , stderr );
	exit( 1 );
	}
