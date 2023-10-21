#include <stdio.h>
#include <math.h>
#include "extern.h"
#include "macros.h"


g_bod (f)
FILE * f;
{
    int     k,
            x;
    double  big = 0.0,
            little = HUGE;
    char    buf[512];


    for (ysue = 0;; ysue++) {
	if (fgets (buf, 512, f) == NULL)
	    break;
	xsue = strlen (buf) - 1;
	for (x = 0; x < xsue; x++) {
	    k = buf[x];
	    suzie[x][ysue] = (double) k;
	    if (big < k)
		big = k;
	    if (little > k)
		little = k;
	}
    }
    big = big - little;
    for (k = 0; k < ysue; k++)
	for (x = 0; x < xsue; x++)
	    suzie[x][k] = (suzie[x][k] - little) / big;
}
