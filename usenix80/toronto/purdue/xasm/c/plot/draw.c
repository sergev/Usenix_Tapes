#include "/v/wa1yyn/c/plot/plot.h"
draw(pstat,x,y)
	int pstat[],x,y;
/*
 *	Draw a vector from current co-ordinate to (x,y).
 */
{
	register i;

	x =& 037777;	y =& 037777;
    if(pstat[CD] == TEK){
	if((pstat[CM] & 1) == 0){
		pstat[CM]++;
		plotp(pstat,GS);
		pstat[CM] =& 0177775;
		pstat[CM] =+ 2;
		plotty(pstat,pstat[Y]);	plottx(pstat,pstat[X]);
		plotfil(pstat,x,y);
		plotty(pstat,y),		plottx(pstat,x);
		pstat[X] = x;	pstat[Y] = y;
		if(pstat[DEBUG]){
			plotp(pstat,US);
			pstat[CM] =& 0177774;
			}
		return;
		}
	if((pstat[CM] & 2) == 0){
		plotty(pstat,pstat[Y]);	plottx(pstat,pstat[X]);
		pstat[CM] =& 0177775;	pstat[CM] =+ 2;
		}
	plotfil(pstat,x,y);
	plotty(pstat,y);	plottx(pstat,x);
	pstat[X] = x;	pstat[Y] = y;
	if(pstat[DEBUG]){
		plotp(pstat,US);	pstat[CM] =& 017774;
		}
	return;
	}

    if(pstat[CD] == HP){
	if((pstat[CM] & 1)== 0){
		plotp(pstat,ETX);
		pstat[CM]=& 0177774;
		pstat[CM] =+ 1;
		}
	if(pstat[CM] & 2){		/* next will draw	*/
		pmbpo(pstat,x,y);
		}
	    else{
		plotp(pstat,'p');
		pmbpo(pstat,pstat[X],pstat[Y]);
		pmbpo(pstat,x,y);
		pstat[CM] =& 0177774;	pstat[CM] =+ 3;
		}
	if(pstat[DEBUG]){
		plotp(pstat,'}');
		plotp(pstat,'~');	plotp(pstat,'\'');
		pstat[CM] =& 0177774;
		}
	pstat[X] = x;	pstat[Y] = y;
	}

}
