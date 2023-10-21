
#include <stdio.h>

#define	buf_size	128
char	buffer[ buf_size ];

main( argc, argv )
int	argc;
char	**argv;
{ /* main */
	extern	FILE	*fopen();
	extern	char	*fgets();

	char	*my_name;

	FILE	*in_phyle;

	FILE	*out_phyle;
	char	out_name[ buf_size ];
	int	out_mode;

	char	*mark;
	char	*ptr;

	argc--;
	my_name = *argv++;

	if( argc < 1 )
	{
		fprintf( stderr, "usage: uudecode file [ file ... ]\n" );
		exit( 1 );
	} /* if */

	while( argc-- )
	{
		if( !( in_phyle = fopen( *argv, "r" )))
			fprintf( stderr, "%s: couldn't open %s\n",
				my_name, *argv );
		else
		{
			fgets( buffer, buf_size, in_phyle );
			while( strncmp( buffer, "begin ", 6 ))
				fgets( buffer, buf_size, in_phyle );
			
			mark = buffer + 6;

			while( *mark != ' ' )	/* Get output file mode */
			{
				out_mode <<= 3;
				out_mode |= (( *mark++ ) - '0' );
			} /* while */

			mark++;			/* Get output file name */
			ptr = out_name;
			while( *mark && ( *mark != '\n' ))
				*ptr++ = *mark++;
			*ptr = '\0';

			if( !( out_phyle = fopen( out_name, "w" )))
				fprintf( stderr, "%s: couldn't open %s\n",
					my_name, out_name );
			else
			{
				printf( "decoding %s(%o)\n",
					out_name, out_mode );

				decode( in_phyle, out_phyle );

				if( fclose( out_phyle ))
				{
					fprintf( stderr,
						"%s: couldn't close %s\n",
						my_name, out_name );
					unlink( out_name );
				} /* if */
			} /* else */
			if( fclose( in_phyle ))
				fprintf( stderr, "%s: couldn't close %s\n",
					my_name, *argv );
		} /* else */
		argv++;
	} /* while */
} /* main */


decode( input, output )
FILE	*input;
FILE	*output;
{ /* decode */
	unsigned char	byte;
	unsigned char	bit;
	unsigned char	c;
	unsigned char	size;

	while( 1 )
	{
		if(( size = fgetc( input ) - ' ' ) <= 0 )
			break;

		if(( byte = fgetc( input )&95 ) >= 64 )
			byte -= ' ';
		bit = 2;

		while( size-- )
		{
			if(( c = fgetc( input )&95 ) >= 64 )
				c -= ' ';

			byte <<= bit;
			bit = 6 - bit;
			byte |= c>>bit;
			fputc( byte, output );

			bit = 8 - bit;

			byte = c;

			if(( bit == 8 ) && size )
			{
				if(( byte = fgetc( input )&95 ) >= 64 )
					byte -= ' ';
				bit = 2;
			} /* if */
		} /* while */
		fgets( buffer, buf_size, input );
	} /* while */
} /* decode */
