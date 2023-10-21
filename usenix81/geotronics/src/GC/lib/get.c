/*
	get -- standard buffered input routine


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

08-May-1981	D A Gwyn	Created.


Usage:
	% cc program.c -lGC	if "-lGC" contains this package.
*/

#include	<stdio.h>


/*
	Get - input data block from stdin and die on error

Example:
	Get( &buffer[0] , 80 );		// reads 80 bytes into buffer
*/

Get( address , size )
	char	*address ;		// address of data buffer
	int	size ;			// number of bytes desired
	{
	if ( fread( address , size , 1 , stdin ) != 1 )
		Error( "input failed" );
	}
