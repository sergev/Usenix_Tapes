/* Copyright    Massachusetts Institute of Technology    1985	*/

#ifndef Xutil_h
#define Xutil_h

#include <InterViews/Xdefs.h>

/* 
 * Bitmask returned by XParseGeometry().  Each bit tells if the corresponding
 * value (x, y, width, height) was found in the parsed string.
 */
#define NoValue		0x0000
#define XValue  	0x0001
#define YValue		0x0002
#define WidthValue  	0x0004
#define HeightValue  	0x0008
#define AllValues 	0x000F
#define XNegative 	0x0010
#define YNegative 	0x0020

unsigned XParseGeometry(char*, Coord&, Coord&, int&, int&);

#endif
