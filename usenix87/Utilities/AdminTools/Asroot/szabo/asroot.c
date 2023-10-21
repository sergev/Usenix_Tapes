#include	<stdio.h>

/*
 *	Asroot	- execute a command with root permissions.
 *		  compile with 'cc -o asroot asroot.c'
 *		  then 'chown root asroot; chmod u+s asroot'.
 *
 *	This program is a convenience for single-user systems,
 *	BUT it is a MASSIVE security hole.  Please use caution.
 */

main( argc, argv )
	int	argc;
	char	**argv;
{
	extern	char	*gets();
	int		retcode;

	char	string[260];

	setuid( geteuid() );

	if ( argc > 1 ) {
		execvp( argv[1], &argv[1] );
		fprintf( stderr,"%s: execution of '%s' failed: ", argv[0], argv[1] );
		perror( "" );
		exit( 1 );
	} else {
		printf("Enter string to be executed: ");
		fflush( stdout );
		retcode=system(  gets( string ) );
		if ( retcode != 0 )
			fprintf(stderr,"%s: the command '%s' exited with status %d\n", argv[0], string, retcode );
		exit( retcode );
	}
}
