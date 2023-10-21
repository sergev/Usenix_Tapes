#include <stdio.h>
#include <errno.h>

FILE * popen();
char * realloc();

int nargsize;
char * narg;

main( argc, argv )
int	argc;
char	* argv[];
	{

	FILE * pfd;
	char buf[BUFSIZ];
	int ch;

	narg = (char *)malloc(BUFSIZ);
	nargsize = BUFSIZ;

	strcpy( narg, "/usr/lib/sendmail -oi -em" );


	while( (fgets( buf, sizeof(buf), stdin ) != NULL)
	&&     (strcmp( buf, "->end\n" ) != 0 ) )
		{
		if( strncmp( buf, "->to:", 5 ) == 0 )
			add_arg( &buf[6] );
		else
		/*  a little cooperation is required here - this comes first */
		if( strncmp( buf, "->from:", 7 ) == 0 )
			{
			add_arg( "-f" );
			add_arg( &buf[8] );
			}
		else
			{
			fprintf( stderr, "illegal control line\n" );
			exit( EINVAL );
			}
			
		}
	
	
	if( (pfd = popen( narg, "w" )) == NULL )
		{
		fprintf( stderr, "couldn't open sendmail\n" );
		exit( EIO );
		}
	
	while( (ch = getchar()) != EOF )
		putc( ch, pfd );
	
	exit( pclose(pfd) );

	}


/*	not very efficient, but easy ... */
add_arg( str )
char * str;
	{

	if( (strlen(narg) + strlen(str) + 2) > nargsize )
		narg = (char *)realloc( narg, (nargsize += BUFSIZ) );

	if( !narg )
		exit( E2BIG );

	strcat( narg, " " );
	strcat( narg, str );
	if( narg[strlen(narg)-1] == '\n' )
		narg[strlen(narg)-1] = '\0';

	}

