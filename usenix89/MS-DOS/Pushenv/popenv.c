#include	<stdio.h>
#include	<stdlib.h>
#include	<direct.h>
#include	"getargs.h"
#include	"masdos.h"

int     Arg_Debug = 0;

ARG Argtab[] =
{
        { 'z', ARG_FBOOLEAN,	&Arg_Debug,			"Debug Mode" }
};

#define ARG_TABSIZE ( sizeof(Argtab) / sizeof(ARG) )

void usage()
{
	fprintf( stderr, "Usage: Popenv [%cZ] env_var\n", ARG_Switch );
	fprintf( stderr, "\n");
	fprintf( stderr, "Set the environment variable SWITCHAR to the character\n");
	fprintf( stderr, "you wish to use for the Switch Character\n");
	fprintf( stderr, "\n" );
	fprintf( stderr, "Case of the command line switches %s important\n",
		 ARG_ICase ? "is not" : "is" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "(C) 1986 Michael A. Shiels\n");
	fprintf( stderr, "\n" );
}

main( argc, argv )
int argc;
char *argv[];
{
	char	ndir[1000], *ndirs = ndir;
	char	dir[1000], *dirs, *p, *dira, *dirb;

	argc = getargs( argc, argv, Argtab, ARG_TABSIZE );
	ctlc();

	if( argc != 2 )
	{
		usage();
		exit(1);
	}

	p = getenv( argv[1] );
	strcpy( (dirs = dir), p );

	if( Arg_Debug )
		fprintf( stderr, "Current %s=%s\n", argv[1], dirs );

	dira = next( &dirs, ';', -1 );

	if( dira == '\0' )
	{
		fprintf( stderr, "PopEnv: Nothing to Pop.\n" );
		exit(1);
	}
	
	dirb = next( &dirs, '\0', -1 );
	if( dirb )
	{
		sprintf( ndirs, "%s=", argv[1] );
		strcat( ndirs, dirb );
	}
	else
		sprintf( ndirs, "%s", argv[1] );
	putdenv(ndir);
	exit();
}
