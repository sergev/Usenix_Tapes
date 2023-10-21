/*
	denude -- strip comments from C program source file

	last edit: 15-Jan-1981  D A Gwyn

Usage:
	% denude <input.c >output.c
*/

#include        <ctype.h>
#include        <stdio.h>


main( argc , argv )
	int     argc ;
	char    *argv[];
	{
	register char   c ;
	register enum   { normal , slash , slsl , slstar , slstst ,
			  dquote , squote , dback , sback }
			state = normal ;

	while ( (c = getchar()) != EOF )
		switch ( state )
			{
		case normal:            // most text
			if ( c == `/' )
				state = slash ;
			else if ( c == `"' )
				{
				putchar( c );
				state = dquote ;
				}
			else if ( c == ``' || c == `\'' )
				{
				putchar( c );
				state = squote ;
				}
			else
				putchar( c );
			continue ;
		case slash:             // / seen
			if ( c == `/' )
				state = slsl ;
			else if ( c == `*' )
				state = slstar ;
			else    {
				putchar( `/' );
				putchar( c );
				state = normal ;
				}
			continue ;
		case slsl:              // // seen
			if ( c == `\n' )
				{
				putchar( `\n' );
				state = normal ;
				}
			continue ;
		case slstar:            // /* seen
			if ( c == `*' )
				state = slstst ;
			continue ;
		case slstst:            // /* ... * seen
			if ( c == `/' )
				{
				putchar( ` ' );
				state = normal ;
				}
			else if ( c != `*' )
				state = slstar ;
			continue ;
		case dquote:            // " seen
			putchar( c );
			if ( c == `\\' )
				state = dback ;
			else if ( c == `"' )
				state = normal ;
			continue ;
		case squote:            // ` or ' seen
			putchar( c );
			if ( c == `\\' )
				state = sback ;
			else if ( c == `\'' )
				state = normal ;
			continue ;
		case dback:             // " ... \ seen
			putchar( c );
			if ( ! isdigit( c ) )
				state = dquote ;
			continue ;
		case sback:             // ` ... \ seen
			putchar( c );
			if ( ! isdigit( c ) )
				state = squote ;
			continue ;
		default:
			fputs( "BUG: illegal state\n" , stderr );
			exit( 1 );
			}
	if ( state != normal )
		fputs( "WARNING: unexpected EOF\n" , stderr );
	exit( 0 );
	}
