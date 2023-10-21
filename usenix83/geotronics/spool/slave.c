/*
	slave -- simple-minded device slave for spooling system


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

09-Oct-1981	D A Gwyn	Created.


Usage:
	Invoked by despooler to transfer file to output device, as
	% slave file device

Method:
	Just a straight file copy; must be modified for funny devices.
*/

#include	<stdio.h>

extern char	_sobuf[BUFSIZ];


main( argc, argv )
	register int	argc;
	register char	*argv[];
	{
	register int	c;

	if ( argc != 3 )
		exit( 1 );		/* Usage: slave file device */

	if ( freopen( argv[1], "r", stdin ) == NULL )
		exit( 2 );		/* no input file */

	if ( freopen( argv[2], "w", stdout ) == NULL )
		exit( 3 );		/* no output device */
	setbuf( stdout, _sobuf );	/* force buffered output */

	while ( (c = getchar()) != EOF )
		putchar( c );		/* copy file to device */

	if ( ferror( stdin ) )
		exit( 4 );		/* read failed */
	if ( ferror( stdout ) )
		exit( 5 );		/* write failed */

	exit( 0 );			/* successful */
	}
