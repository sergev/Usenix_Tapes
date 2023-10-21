#include <stdio.h>

main(argc, argv)
	int argc;
	char **argv;
{
	register int i, nflg, rflg;

	nflg = rflg = 0;
	while ( argc > 1 && argv[ 1 ][ 0 ] == '-' ) {
		i = 0;
		while ( argv[ 1 ][ ++i ] ) {
			switch ( argv[ 1 ][ i ] ) {

			case '-':
				argc--;
				argv++;
				goto skip;

			case 'q':
				exit( 0 );

			case 'n':
				nflg++;
				break;

			case 'r':
				rflg++;
				srand( getpid() );
				break;

			default:
				fputs( "usage: show [-qnr] string\n", stderr );
				exit( 2 );

			}
		}

		argc--;
		argv++;
	}

skip:
	for ( i = 1; i < argc; i++ ) {
		if ( ( ( ! rflg ) || ( rflg && ( rand() & 010 ) ) ) ) {
			output( argv[i] );
			if ( i < argc-1 )
				putchar( ' ' );
		}
	}
	if ( ! nflg )
		putchar( '\n' );

	exit( 0 );
}

output( string )
	char *string;
{
	int val;
	char c;

	while ( *string ){
		if ( *string == '\\' ) {
			string++;

			switch ( *string++ ) {

			case '\0':
				break;

			case 'a':
				putchar( '\007' );
				break;

			case 'b':
				putchar( '\010' );
				break;

			case 'c':
				exit( 0 );

			case 'f':
				putchar( '\014' );
				break;

			case 'n':
				putchar( '\n' );
				break;

			case 'r':
				putchar( '\r' );
				break;

			case 't':
				putchar( '\t' );
				break;

			case 'v':
				putchar( '\013' );
				break;

			case 'x':
				if ( *string == '\0' )
					break;
				val = hexconvert( *string++ );
				if ( ( *string >= '0' && *string <= '9' ) ||
				    ( *string >= 'a' && *string <= 'f') ||
				    ( *string >= 'A' && *string <= 'F') ) {
					val = (val<<4)+hexconvert( *string++ );
				}
				putchar( (char)val );
				break;

			default:
				string--;
				if ( *string >= '0' && *string <= '7' ) {
					val = (int)( *string++ - '0' );
					if ( *string >= '0' && *string <= '7' ) {
						val = (val<<3)+(int)( *string++ - '0' );
						if ( *string >= '0' && *string <= '7' ) {
							val = (val<<3)+(int)( *string++ - '0' );
						}
					}
					putchar( (char)val );
				} else
					putchar( *string++ );
			}
		} else {
			putchar( *string++ );
		}
	}
}

hexconvert( c )
	char c;
{
	if ( c >= '0' && c <= '9' )
		return (int)( c - '0' );
	else if ( c >= 'a' && c <= 'f' )
		return (int)( c - 'a' + 10 );
	else if ( c >= 'A' && c <= 'F' )
		return (int)( c - 'A' + 10 );
	else
		return 0;
}
