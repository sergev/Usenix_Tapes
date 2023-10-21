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
	fprintf( stderr, "Usage: pushd [%cZ] [directory]\n", ARG_Switch );
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
	char	cwd[80], *p;
	int		error;

	argc = getargs( argc, argv, Argtab, ARG_TABSIZE );
	ctlc();

	insint24();

	if( argc != 2 )
	{
		usage();
		exit(1);
	}

	dirs = getenv("PUSHPOP");

	if( Arg_Debug )
		fprintf( stderr, "Current PUSHPOP=%s\n", getenv("PUSHPOP") );
	
	getcwd( cwd, 1000 );
	for( p = cwd ; *p ; p++ )
		if( *p == '\\' )
			*p = '/';

	strcpy( ndirs, "PUSHPOP=" );
	strcat( ndirs, cwd );
	strcat( ndirs, ";" );
	if( dirs )
		strcat( ndirs, dirs );

	error = cd(argv[1]);
	if( !error )
	{
		error = int24err();
		if( !error )
		{
			putdenv(ndir);
			fprintf( stderr, "Pushing to %s.\n", argv[1] );
			exit();
		}
		else
		{
			cd(cwd);
			fprintf( stderr, "Pushd: Error CDing to directory %s.", argv[1] );
			exit(1);
		}
	}
	else
	{
		fprintf( stderr, "Pushd: Error CDing to directory %s.", argv[1] );
		exit(1);
	}
}
