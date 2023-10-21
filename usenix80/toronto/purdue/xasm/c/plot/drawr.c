#include "/v/wa1yyn/c/plot/plot.h"
drawr(pstat,deltax,deltay)
	int pstat[],deltax,deltay;
/*
 *	Draw to relative postion given by delta-x and delta-y.
 */
{
	register x,y;

	x = deltax + pstat[X];
	while(x < 0)x =+ 10230;
	if(x > 10230)x =% 10230;
	y = deltay + pstat[Y];
	while(y < 0)y =+ 7790;
	if(y > 7790)y =% 7790;
	draw(pstat,x,y);
}
