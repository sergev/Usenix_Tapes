#include "/v/wa1yyn/c/plot/plot.h"
drawvr(pstat,dx1,dy1,dx2,dy2)
	int pstat[],dx1,dy1,dx2,dy2;
/*
 *	Draw vector with start and stop points relative to current
 *	postion.
 */
{
	register x,y,x2;
	int y2;

	x = dx1 + pstat[X];
	while(x < 0)x =+ 10230;
	if(x > 10230)x =% 10230;
	y = dy1 + pstat[Y];
	while(y < 0)y =+ 7790;
	if(y > 7790)y =% 7790;

	x2 = dx2 + pstat[X];
	while(x2 < 0)x2 =+ 10230;
	if(x2 > 10230)x2 =% 10230;
	y2 = dy2 + pstat[Y];
	while(y2 < 0)y2 =+ 7790;
	if(y2 > 7790)y2 =% 7790;

	vector(pstat,x,y,x2,y2);
}
