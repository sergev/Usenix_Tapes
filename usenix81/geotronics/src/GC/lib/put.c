/*
	put -- standard buffered output routine


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

08-May-1981	D A Gwyn	Created.


Usage:
	% cc program.c -lGC	if "-lGC" contains this package.
*/

#include	<stdio.h>


/*
	Put - output data block to stdout and die on error

Example:
	Put( &buffer[0] , 80 );		// writes 80 bytes from buffer
*/

Put( address , size )
	char	*address ;		// address of data buffer
	int	size ;			// number of bytes desired
	{
	if ( fwrite( address , size , 1 , stdout ) != 1 )
		Error( "output failed" );
	}
