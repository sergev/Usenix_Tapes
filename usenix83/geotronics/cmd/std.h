/*
	<std.h> -- standard header for Geotronics Corporation C programs


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

09-May-1981	D A Gwyn	Created.
30-Nov-1981	D A Gwyn	Added DEGRAD.
09-Jan-1982	D A Gwyn	Added NULL.
12-Jan-1982	D A Gwyn	Added `character', EOS.
01-Mar-1982	D A Gwyn	Added Round().
17-Mar-1982	D A Gwyn	Added `void'.
01-May-1982	D A Gwyn	`void' removed for new compiler.


Usage:
	#include	<math.h>	(if you invoke Round())
	#include	<std.h> 	in your C program source.

Note:
	This file is maintained by the Manager of Software Development.
*/

/* Extended data types */

#ifndef NULL
#define NULL	(char *)0		/* null pointer value */
#endif

typedef int	bool;			/* boolean data type */
#define 	NO	0
#define 	YES	1

typedef short	character;		/* character data type used
					   to hold chars + EOF etc. */
#define 	EOS	'\0'		/* end-of-string marker */

/* typedef int	void;			/* non-value function type;
					   remove for UNIX System III */

/* Universal constants */

#define DEGRAD	57.2957795130823208767981548141051703324054724665642
					/* degrees per radian */
#define LOG10E	0.43429448190325182765112891891660508229439700580367
#define PI	3.14159265358979323846264338327950288419716939937511

/* Useful macros */

#define Abs( a )	((a) >= 0 ? (a) : -(a))
#define Max( a, b )	((a) >= (b) ? (a) : (b))
#define Min( a, b )	((a) >= (b) ? (b) : (a))
#define Round( dbl )	floor( (dbl) + 0.5 )
