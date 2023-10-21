/*
 *	hungry.c -- set/display hunger status
 *	Phil Budne @ Boston University / Distributed Systems
 */

# include <stdio.h>
# include <sys/types.h>
# include <sys/stat.h>

# include "hungry.h"

main( argc, argv )
int argc;
char *argv[];
{
    struct stat stb;
    int f;

    f = fileno( stderr );
    if( !isatty( f ) ) {
	fprintf(stderr, "?no tty\n");
	exit( 2 );
    } /* not a tty */

    if( fstat( f, &stb ) < 0 ) {
	perror( "fstat" );
	exit( 2 );
    } /* fstat failed */

    if( argc < 2 ) {
	if( (stb.st_mode & HUNGRY) != 0 )
	    puts("is y");
	else
	    puts("is n");
    } /* argc < 2 */
    else {
	int newmode;

	newmode = stb.st_mode & ~S_IFMT; /* clear file format bits */

	switch( argv[1][0] ) {
	case 'y':
	    newmode |= HUNGRY;
	    break;

	case 'n':
	    newmode &= ~HUNGRY;
	    break;

	case '^':			/* toggle! */
	    newmode ^= HUNGRY;
	    break;

	default:
	    fprintf(stderr, "%s [y n ^]\n", argv[0] );
	    exit( 2 );
	} /* switch */

# ifdef sun			/* avoid sun bug */
	{
	    char *name;
	    extern char *ttyname();

	    if( (name = ttyname( f )) == NULL ) {
		perror( "ttyname" );
		exit( 2 );
	    } /* ttyname failed */

	    if( chmod(name, newmode) < 0 )
		perror( "chmod" );
	} /* sun */
# else
	if( fchmod(f, newmode) < 0 )
	    perror( "fchmod" );
# endif not sun

    } /* argv >= 2 */

    if( (stb.st_mode & HUNGRY) == 0 )
	exit( 1 );		/* false */
    else
	exit( 0 );		/* true */
} /* main */
