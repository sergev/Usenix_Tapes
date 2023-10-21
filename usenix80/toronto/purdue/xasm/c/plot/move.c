#include "/v/wa1yyn/c/plot/plot.h"
move(pstat,x,y)
	int pstat[],x,y;
/*	
 *	Move to an x,y co-ordinate.  Leave in graphic mode.
 */
{

	x =& 037777;	y =& 037777;
    if(pstat[CD] == TEK){
	if((pstat[CM] & 1) == 0){
		plotp(pstat,GS);
		pstat[CM]++;
		pstat[CM] =& 0177775;	pstat[CM] =+ 2;
		plotty(pstat,y);	plottx(pstat,x);
		pstat[X] = x;	pstat[Y] = y;
		if(pstat[DEBUG]){
			plotp(pstat,US);
			pstat[CM] =& 0177774;
			}
		return;
		}
	if((pstat[CM] & 2) == 2){
		plotp(pstat,GS);
		}
	plotty(pstat,y);	plottx(pstat,x);
	pstat[CM] =& 0177775;	pstat[CM] =+ 2;
	pstat[X] = x;	pstat[Y] = y;
	if(pstat[DEBUG]){
		plotp(pstat,US);	pstat[CM] =& 0177774;
		}
	return;
	}

    if(pstat[CD] == HP){
	if((pstat[CM] & 1) == 0){
		plotp(pstat,ETX);	/* end alpha mode	*/
		pstat[CM] =& 0177774;
		}
	if(pstat[CM] & 2){
		plotp(pstat,'}');
		}
	plotp(pstat,'p');
	pmbpo(pstat,x,y);
	if(pstat[DEBUG]){
		plotp(pstat,'}');	plotp(pstat,'~');
		plotp(pstat,'\'');
		pstat[CM] =& 0177774;
		}
	    else{
		pstat[CM] =| 3;
		}
	pstat[X] = x;	pstat[Y] = y;
	}

}
