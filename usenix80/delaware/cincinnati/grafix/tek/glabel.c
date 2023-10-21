#include "gchars.h"	/* get character vectors etc. */


extern int ox, oy;

glabel(str, rot, scale)
	char *str;
	double rot, scale;
{
	double dmax(), sin(), cos(), xfac, yfac;
	int i, cx, cy, x, y, x0, y0, pen;

	rot =* 3.1415926/180.;	/* convert to radians */

	xfac = cos(rot) * scale;
	yfac = sin(rot) * scale;	/* adjustments... */

	x0 = dmax(ox - (xdim*xfac/2.),0.);	/* put center of char on point */
	y0 = dmax(oy - (ydim*yfac/2.),0.);

	for(;*str;str++){
		penup();	/* pen == 0 -> up */
		pen = 0;	/* pen == 1 -> down */
		for(i = vecptrs[*str - 32] + 1;(cx=charvec[i][0]) != -2; i++){
			if(cx == -1){
				penup();
				pen = 0;
				continue;
			}
			cy =  charvec[i][1];
			x = x0 + cx*xfac - cy*yfac + .5;
			y = y0 + cx*yfac + cy*xfac + .5;
			pt(x,y);
			if(!pen){
				pen++;
				pendown();
			}
		}
		/* reset origin to next position... */
		x0 =+ xdim*xfac;
		y0 =+ ydim*yfac;
		penup();
		pen = 0;
		pt(x0,y0);
	}
}

double dmax(a,b)
	double a,b;
{
	return(a>b?a:b);
}
