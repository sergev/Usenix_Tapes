#include	<stdio.h>

char	text	[512];
char	name	[128];
char	*pnt0;
char	*pnt1;
char	*index	();

FILE	*input;
FILE	*output;

main ()
{
	input = fopen ( "primek.plp" , "r" );
	if ( input == NULL )
	{
		printf ( "Can not find primek.plp\n" );
		exit ( 1 );
	}
	output = fopen ( "junk" , "w" );
	if ( output == NULL )
	{
		printf ( "Can not use this directory\n" );
		exit ( 2 );
	}

	while ( fgets(text,sizeof(text),input) != NULL )
	{
		if ( !strncmp ( text , ":::::" , 5 ) )
		{
			if (fgets(name,sizeof(name),input)==NULL) break;
			pnt0 = index ( name , '*' );
			pnt0++;
			pnt1 = index ( pnt0 , '*' );
			*pnt1 = 0;
			printf ( "Extracting file >>%s<<\n" , pnt0 );
			fclose ( output );
			output = fopen ( pnt0 , "w" );
		}
		else
		{
			fprintf ( output , "%s" , text );
		}
	}
	fclose ( output );
	fclose ( input );
	printf ( "Prime Kermit Extraction Complete\n" );
}
