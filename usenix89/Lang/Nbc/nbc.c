
/*  nbc.c	v 1.0.00	11-mar-87	rpk
 *
 *  remove first block comment from a source file
 */

#include <stdio.h>

main( argc, argv)
int argc; char *argv[];
{
	FILE *fp;
	char ch, buffer[BUFSIZ];
	;
	if ( (fp = argc < 2 ? stdin : fopen( argv[1], "r" )) == NULL ) {
	    perror( argv[1] );
	    exit( 1 );
	    }
	while ( fgets( buffer, BUFSIZ, fp ) ) {
	    if ( strncmp( buffer, "/*******", 8 ) == 0 ) {
		while ( fgets( buffer, BUFSIZ, fp ) ) {
		    if ( strncmp( buffer, " *******", 8 ) == 0 )
			goto zip;
		    }
		}
	    else {
		fputs( buffer, stdout );
		}
	    }
zip:
	while ( (ch = getc( fp )) != EOF )
	    putchar( ch );

	exit( 0 );
}
