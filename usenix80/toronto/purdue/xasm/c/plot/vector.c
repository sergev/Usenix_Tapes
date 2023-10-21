#include "/v/wa1yyn/c/plot/plot.h"
vector(pstat,x1,y1,x2,y2)
	int pstat[],x1,x2,y1,y2;
/*
 *	Draws vector from (x1,y1) to (x2,y2)
 */
{
	x1 =& 037777;	x2 =& 037777;
	y1 =& 037777;	y2 =& 037777;
    if(pstat[CD] == TEK){
	if((pstat[CM] & 1)== 0){
		plotp(pstat,GS);
		pstat[CM]++;
		pstat[CM] =& 0177775;
		}
	if((pstat[CM] & 2) == 2){
		if((x1 == pstat[X]) && (y1 == pstat[Y])){
			plotfil(pstat,x2,y2);
			plotty(pstat,y2);	plottx(pstat,x2);
			pstat[X] = x2;	pstat[Y] = y2;
			if(pstat[DEBUG]){
				goalpha(pstat);
				}
			return;
			}
		move(pstat,x1,y1);
		draw(pstat,x2,y2);
		return;
		}
	plotty(pstat,y1);	plottx(pstat,x1);
	pstat[X] = x1;	pstat[Y] = y1;
	plotfil(pstat,x2,y2);
	plotty(pstat,y2);	plottx(pstat,x2);
	pstat[X] = x2;	pstat[Y] = y2;
	pstat[CM] =+ 2;
	if(pstat[DEBUG]){
		goalpha(pstat);
		}
	return;
	}

    if(pstat[CD] == HP){
	if((x1 != pstat[X])||(y1 != pstat[Y]))move(pstat,x1,y1);
	draw(pstat,x2,y2);
	}
}
