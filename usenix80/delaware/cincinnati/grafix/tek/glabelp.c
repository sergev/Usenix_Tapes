#include "gchars.h"	/* get character vectors etc. */
#define	max(a,b)	(a>b?a:b)


extern int ox, oy;

glabel(str, rot, scale)
	char *str;
	double rot, scale;
{
	double sin(), cos(), xfac, yfac;
	int i, x, y, x0, y0, pen, xmax, ymax;
	register cx, cy;

	rot =* 3.1415926/180.;	/* convert to radians */

	xfac = cos(rot) * scale;
	yfac = sin(rot) * scale;	/* adjustments... */

	x0 = max(ox - (xdim*xfac/2.),0.);	/* put center of char on point */
	y0 = max(oy - (ydim*yfac/2.),0.);

	for(;*str;str++){
		penup();	/* pen == 0 -> up */
		pen = 0;	/* pen == 1 -> down */
		xmax = ymax = 0;
		for(i = vecptrs[*str - 32] + 1;(cx=charvec[i][0]) != -2; i++){
			if(cx == -1){
				penup();
				pen = 0;
				continue;
			}
			cy =  charvec[i][1];
			x = x0 + cx*xfac - cy*yfac + .5;
			y = y0 + cx*yfac + cy*xfac + .5;
			xmax = max(cx, xmax);	/* proportional spacing! */
			ymax = max(cy, ymax);
			pt(x,y);
			if(!pen){
				pen++;
				pendown();
			}
		}
		/* reset origin to next position... */
		x0 =+ xmax*xfac;
		y0 =+ ymax*yfac;
		penup();
		pen = 0;
		pt(x0,y0);
	}
}

