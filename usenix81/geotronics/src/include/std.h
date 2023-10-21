/*
	std.h -- standard header for all C programs


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

09-May-1981	D A Gwyn	Created.

Usage:
	#include	<std.h> 	in your C program source.

Note:
	This file is maintained by the Manager of Software Development.
*/

// Extended data types

typedef int	bool ;			// logical data type
#define 	NO	0
#define 	YES	1

// Universal constants

#define	LOG10E	0.43429448190325182765112891891660508229439700580367
#define	PI	3.14159265358979323846264338327950288419716939937511

// Useful macros

#define	Abs( a )	((a) >= 0 ? (a) : - (a))
#define	Max( a , b )	((a) >= (b) ? (a) : (b))
#define	Min( a , b )	((a) >= (b) ? (b) : (a))
