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
	fprintf( stderr, "Usage: popd [%cZ]\n", ARG_Switch );
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
	char	cwd[80], *pa;
	int		error;

	argc = getargs( argc, argv, Argtab, ARG_TABSIZE );
	ctlc();

	insint24();

	if( argc != 1 )
	{
		usage();
		exit(1);
	}

	p = getenv( "PUSHPOP" );
	strcpy( (dirs = dir), p );

	if( Arg_Debug )
		fprintf( stderr, "Current PUSHPOP=%s\n", dirs );

	dira = next( &dirs, ';', -1 );

	if( dira == '\0' )
	{
		fprintf( stderr, "Popd: No directory to Pop to.\n" );
		exit(1);
	}
	
	getcwd( cwd, 1000 );
	for( pa = cwd ; *pa ; pa++ )
		if( *pa == '\\' )
			*pa = '/';

	error = cd( dira );
	if( !error )
	{
		error = int24err();
		if( !error )
		{
			dirb = next( &dirs, '\0', -1 );
			strcpy( ndirs, "PUSHPOP=" );
			strcat( ndirs, dirb );
			putdenv(ndir);
			fprintf( stderr, "Poping to %s\n", dira );
			exit();
		}
		else
		{
			cd(cwd);
			fprintf( stderr, "Popd: Error CDing to directory %s.", dira );
			exit(1);
		}
	}
	else
	{
		fprintf( stderr, "Popd: Error CDing to directory %s.", dira );
		exit(1);
	}
}
