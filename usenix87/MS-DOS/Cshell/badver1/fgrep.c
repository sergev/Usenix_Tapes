#include <stdio.h>
#include <fcntl.h>

#include <signal.h>
#include <setjmp.h>

void (*signal())();
void (*fgrepsig)();
jmp_buf fgrep_env;

void fgrep_intr()
{
	/* restore old signal */
	signal(SIGINT,fgrepsig);
	/* jump to exit */
	longjmp(fgrep_env,-1);
}

FILE *fopen(),*fdopen();

char *fgets();

fgrep( argc, argv )
int	argc;
char * argv[];
	{
	FILE *  fd;
	int	rc;
	char * pattern;
	/* handle interrupts */
	if (-1 == setjmp(fgrep_env))
	{
		static char *intmsg = "Interrupted\r\n";
		write(2,intmsg,strlen(intmsg));
		fclose(fd);
		return -1;
	}
	/* set signal catcher */
	fgrepsig= signal(SIGINT,fgrep_intr);
	while( --argc )
		{
		if( (*++argv)[0] == '-' )
			switch( (*argv)[1] )
				{
				default:
					fprintf( stderr, "invalid argument %s\n", *argv );
					return(1);
				}
		else
			break;
		}

	if( argc == 0 )
		{
		fprintf( stderr, "usage: fgrep pattern [file ...]\n" );
		return(1);
		}

	pattern = *argv++;
	argc--;

	rc = 0;
	if( argc == 0 )
		{
		fd = fdopen(0,"r");
		rc = _fgrep( NULL, pattern, fd);
		fclose(fd);
		return( 0 );
		}
	else
	while( argc-- )
		{

		if( (fd = fopen( *argv, "r" )) == NULL )
			fprintf( stderr, "couldn't open %s\n", *argv );
		else
			{
			rc |= _fgrep( *argv, pattern, fd );
			fclose( fd );
			}
		argv++;

		}

	return( !rc );
	signal(SIGINT,fgrepsig);
	}


_fgrep( file, pattern, fd )
char	* file;
char	* pattern;
FILE *	fd;
	{
	char	line[BUFSIZ];
	int		rc;
	int		linenumber = 1;
	rc = 0;
	while( fgets( line, sizeof(line), fd ) != NULL )
	{
		if( rc = match( pattern, line ) )
			printf( "%s %d: %s", (file) ? file : "stdin", linenumber, line );
		linenumber++;
	}
	return( rc );

	}

match( pattern, line )
register char	* pattern;
char	* line;
	{
	/*  not a great algorithm  */
	register char * ptr;
	char	* end;
	int	plen = strlen(pattern);
	int llen = strlen(line);

	if( plen > llen )
		return( 0 );

	end = line+(llen-plen);

	for( ptr=line; ptr < end; ptr++ )
		{
		if( strncmp( pattern, ptr, plen ) == 0 )
			return( 1 );
		}

	return( 0 );
	}
