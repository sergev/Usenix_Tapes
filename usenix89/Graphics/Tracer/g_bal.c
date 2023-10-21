#include <stdio.h>
#include "rtd.h"
#include "extern.h"
g_bal (df)/* gets data for the balls, returns number of balls */
FILE * df;
{
    int     i;
    double  x,
            y,
            z,
            r,
            ior,
            rfr,
            rfl,
            dif,
            amb;
    for (i = 0;
	    fscanf (df, "%F %F %F %F %F %F %F %F %F",
		&x, &y, &z, &r, &ior, &rfr, &rfl, &dif, &amb) != EOF;
	    i++) {
/* get some space for the ball */
	bl[i] = (struct ball   *) malloc (sizeof (struct ball));
	bl[i] -> s.cent.x = x;
	bl[i] -> s.cent.y = y;
	bl[i] -> s.cent.z = z;
	bl[i] -> s.rad = r;
	bl[i] -> ior = ior;
	bl[i] -> rfr = rfr;
	bl[i] -> rfl = rfl;
	bl[i] -> dif = dif;
	bl[i] -> amb = amb;
    }
    return (i);
}

