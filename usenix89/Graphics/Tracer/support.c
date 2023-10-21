/*
 *    supportive subroutines...
 */

#include <math.h>
#include <stdio.h>
#include "rtd.h"
#include "extern.h"


mt (vec, trans)/* make a transpose matrix according to arcane laws 
		  hidden in a linear algebra book plus a lot of my own
		  sweat. (if you reaaly wanna know, rotate around y axis,
		  then z axis.) */
struct vector  *vec;
struct mat *trans;
{
    if (vec -> xzl == 0.0) {
	trans -> x.x = 0.0;
	trans -> x.y = 1.0;
	trans -> x.z = 0.0;
	trans -> y.x = -1.0;
	trans -> y.y = 0.0;
	trans -> y.z = 0.0;
	trans -> z.x = 0.0;
	trans -> z.y = 0.0;
	trans -> z.z = 1.0;
    }
    else {
	trans -> x.x = (vec -> x) / (vec -> l);
	trans -> x.y = (vec -> y) / (vec -> l);
	trans -> x.z = (vec -> z) / (vec -> l);
	trans -> y.x = -(vec -> x) * (vec -> y) / ((vec -> l) * (vec -> xzl));
	trans -> y.y = (vec -> xzl) / (vec -> l);
	trans -> y.z = -(vec -> z) * (vec -> y) / ((vec -> l) * (vec -> xzl));
	trans -> z.x = -(vec -> z) / (vec -> xzl);
	trans -> z.y = 0;
	trans -> z.z = (vec -> x) / (vec -> xzl);
    }
}
