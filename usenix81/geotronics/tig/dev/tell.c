/*
	tell -- TIG-pack debugging plot code interpreter

	last edit: 23-Jan-1981	D A Gwyn

Usage:
	% tell <TIGfile >dump
*/

#include	<stdio.h>
#include	<TIGcolors.h>

typedef struct
	{
	unsigned	x ;
	unsigned	y ;
	}	coords ;

static char	*color[] =
	{
	"EOF" ,
	"WHITE" ,
	"CYAN" ,
	"MAGENTA" ,
	"BLUE" ,
	"YELLOW" ,
	"GREEN" ,
	"RED" ,
	"BLACK"
	};


main()
	{
	char	c ;			// input character
	coords	xy ;

	while ( (c = getchar()) != EOF )
		switch ( c )
			{
		case `F':		// new display frame
			printf( "\n\nNew Frame\n" );
			break ;

		case `U':		// raise pen
		case `D':		// lower pen
			printf( "Pen %s\n" , c==`U' ? "Up" : "Down" );
			break ;

		case `C':		// new color & intensity
			printf( "Color=%s, " , color[getchar()+1] );
			printf( "Intensity = %d\n" , getchar() );
			break ;

		case `M':		// move pen
			fread( &xy , sizeof xy , 1 , stdin );
			printf( "Move to (%u,%u)\n" , xy );
			break ;

		default:
			printf( "Illegal code `%c'\n" , c );
			}

	printf( "EOF\n" );
	exit( 0 );
	}
