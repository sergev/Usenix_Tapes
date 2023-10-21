#include	<stdio.h>
#include	<sysexits.h>

/*
 *	rsmail:  send mail to a remote mail "catcher" which will, in turn
 *   invoke a local mail delivery program.  Send mail using the fabulous
 *   uucp facility.
 */

FILE * popen();

main( argc, argv )
int	argc;
char	* argv[];
	{

	FILE * pfd;
	char	rmbuf[BUFSIZ];
	int ch;

	if( argc < 4 )
		{
		fprintf( stderr, "rsmail: too few arguments\n" );
		exit( 1 );
		}

	argc--;
	argv++;

	/*  yuk */
	sprintf( rmbuf, "uux -r -n - %s!rgmail", *argv );

	if( (pfd = popen( rmbuf, "w" )) == NULL )
		{
		fprintf( stderr, "rsmail: couldn't open uux\n" );
		exit(1);
		}

	while( --argc )
		{
		if( (*++argv)[0] == '-' )
			{
			switch( (*argv)[1] )
				{
				case 'f':
					if( --argc == 0 )
						goto badopt;
					fprintf( pfd, "->from: %s\n", *++argv );
					break;
				default:
				badopt:
					fprintf( stderr, "illegal option\n" );
					pclose(pfd);
					exit(1);
				}
			}
		else
			fprintf( pfd, "->to: %s\n", *argv );
		}
	
	fprintf( pfd, "->end\n" );

	while( (ch = getchar()) != EOF )
		putc( ch, pfd );
	
	/*  if you have trouble writing to /usr/spool, try again later. */
	if( ferror(stdin) || ferror(pfd) )
		exit( EX_TEMPFAIL );

	exit( pclose( pfd ) );

	}

