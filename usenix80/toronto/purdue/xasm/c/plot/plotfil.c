#include "/v/wa1yyn/c/plot/plot.h"
plotfil(pstat,x,y)
	int pstat[],x,y;
/*
 *	Look at magnitude of change in current X andY and
 *	new x and y.  Set correct number of fill charctors.
 */
{

	x =- pstat[X];	if(x < 0) x =* -1;
	y =- pstat[Y];	if(y < 0) y =* -1;
	x =& 070000;	y =& 070000;
	x =>> 12;	y =>> 12;
	pstat[FILL] = x;	pstat[FILL] =+ y;
	pstat[FILL]++;	/* for good measure */
}
