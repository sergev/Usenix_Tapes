#include "/v/wa1yyn/c/plot/plot.h"
point(pstat,x,y)
	int pstat[],x,y;
/*
 *	Put a point at point (x,y).
 */
{
	register i,j;

	x =& 037777;	y =& 037777;
    if(pstat[CD] == TEK){
	if((pstat[CM] & 1) == 0){	/* current mode is alpha */
		pstat[CM]++;
		pstat[CM] =& 0177775;	pstat[CM] =+ 2;
		plotp(pstat,GS);
foo:		plotty(pstat,y);	plottx(pstat,x);
bar:		i = x % 10;
		x =/ 10;
		x =& 037;
		if(i > 4)x++;
		plotp(pstat,x + 0100);
		pstat[X] = x;	pstat[Y] = y;
		if(pstat[DEBUG]){
			plotp(pstat,US);	pstat[CM] =& 0177774;
			}
		return;
		}
	if((pstat[CM] & 2) == 0){	/* next will not print */
		goto foo;
		}
	if((x == pstat[X]) && (y == pstat[Y])){
		goto bar;
		}
	}


    if(pstat[CD] == HP){
	if((pstat[CM] & 1)== 0){
		plotp(pstat,ETX);
		pstat[CM] =& 0177774;	pstat[CM] =+ 1;
		}
	if(pstat[CM] & 2){
		plotp(pstat,'}');
		pstat[CM] =& 0177774;	pstat[CM] =+ 1;
		}
	plotp(pstat,'p');
	pmbpo(pstat,x,y);	pmbpo(pstat,x,y);
	pstat[CM] =| 3;
	if(pstat[DEBUG]){
		plotp(pstat,'}');
		plotp(pstat,'~');	plotp(pstat,'\'');
		pstat[CM] =& 0177774;
		}
	}
}
