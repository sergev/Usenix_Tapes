/*
**  Split stdin into FILESIZE (200K byte) chunks, named Part-a .. Part-b
**
**  Handy for splitting up multi-megabyte files into uucp-edible parts.
**
**  Dave Brower, 4/86
**  {sun, amdahl, mtxinu}!rtech!daveb
**
**  Usage:  bustup [ outname ] < bigfile
**
**  where:	outname 	is an optional name to use as the base
**				of the new files.  It defaults to
**				"Part", resulting in files named
**				"Part-a," "Part-b" etc.
**
**  Bugs:  Can only write the 26 parts "a..z".
**         Output file size should be an argument -nnnnn
**
**  The canonical example is:
**
**	cd /usr/spool/uucppublic
**	(cd /usr/local/emacs; tar cf - . ) | compress | bustup gnumacs
**	uucp gnumacs-? othersys!/usr/spool/uucppublic
**
**  And on the other end;
**
**	cd /usr/spool/uucppublic
**	cat gnumacs-? | uncompress | (cd /usr/local/emacs; tar xf -)
*/

# include <stdio.h>

# define FILESIZE 200000

char template[ 100 ] = { 0 };
char fname[ 100 ] = { 0 };

main(argc, argv)
int argc;
char **argv;
{
	register FILE * fp;
	register int i;
	register int byte;
	register int c;

	/* figure the output file name template */

	(void) sprintf( template, "%s-%%c", argc == 2 ? *++argv : "Part" );

	/* read from stdin, creating new files as needed */

	for( c = i = 0; c != EOF ; i++ )
	{
		/* Check for input file too big */

		if( i + 'a' > 'z' )
		{
			(void)fprintf(stderr, "Out of filenames -- input too big\n" );
			exit( 1 );
		}

		/* open a new file */

		(void)sprintf( fname, template, i + 'a' );
		if ( NULL == ( fp = fopen( fname, "w" ) ) )
		{
			(void)fprintf( stderr, "Can't open file %s\n", fname );
			exit ( 1 );
		}

		/* write partial file */

		(void)fprintf( stderr, "Writing file \"%s\"\n", fname );

		for( byte = 0; byte < FILESIZE; byte++ )
		{
			if( ( c = getchar() ) == EOF )
				break;

			putc( c, fp );
		}
		(void)fclose( fp );
	}
	exit( 0 );
}
