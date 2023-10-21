#include "/v/wa1yyn/c/plot/plot.h"
goalpha(pstat)
	int pstat[];
/*
 *	Puts device into alha mode.
 */
{
    if(pstat[CD] == TEK){
	pstat[CM] =& 0177774;
	plotp(pstat,US);
	}
    if(pstat[CD] == HP){
	if((pstat[CM] & 1)== 0)return;
	plotp(pstat,'}');
	plotp(pstat,'~');
	plotp(pstat,'\'');
	pstat[CM] =& 0177774;
	}
}
