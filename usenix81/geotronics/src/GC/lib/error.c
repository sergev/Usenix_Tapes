/*
	error -- standard error-logging routines


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

08-May-1981	D A Gwyn	Created.


Usage:
	% cc program.c -lGC	if "-lGC" contains this package.

Notes:
	Don't forget to call "ErrName()" early in the main program.
*/

#include	<stdio.h>
/*
	ErrName - set program name to appear in error message

Example:
	ErrName( "EdSmoo" );		// sets name to "EdSmoo"
*/

static char	*progname = "*" ;	// defuault name

ErrName( newname )
	char	*newname ;		// new program name
	{
	progname = newname ;
	}


/*
	Error - output error message on stderr then die

Example:
	Error( "oops" );		// prints something like:
					// BEEP * EdSmoo * oops
*/

Error( message )
	char	*message ;		// error message text
	{
	fputs( "\07* " , stderr );	// avoid "fprintf"
	fputs( progname , stderr );
	fputs( " * " , stderr );
	fputs( message , stderr );
	putc( `\n' , stderr );
	exit( 1 );			// fatal error
	}
