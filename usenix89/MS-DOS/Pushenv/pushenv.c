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
	fprintf( stderr, "Usage: PushEnv [%cZ] env_var string_to_add\n", ARG_Switch );
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
	char	*dirs;

	argc = getargs( argc, argv, Argtab, ARG_TABSIZE );
	ctlc();

	if( argc != 3 )
	{
		usage();
		exit(1);
	}

	dirs = getenv(argv[1]);

	if( Arg_Debug )
		fprintf( stderr, "Current %s=%s\n", argv[1], getenv(argv[1]) );
	
	sprintf( ndirs, "%s=", argv[1] );
	strcat( ndirs, argv[2] );
	if( dirs )
	{
		strcat( ndirs, ";" );
		strcat( ndirs, dirs );
	}

	putdenv(ndir);
	exit();
}
