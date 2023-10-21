#include "/v/wa1yyn/c/plot/plot.h"
plotc(pstat,c)
	int pstat[];
	char c;
/*
 *	Print out charactor 'c' on terminal and update X and Y
 *	to show correct current postion.
 */
{
	register i,j;

    if(pstat[CD] == TEK){
	if((pstat[CM] & 1) == 1){
		plotp(pstat,US);	pstat[CM] =& 0177774;
		}
plotclop: plotp(pstat,c);	movec(pstat,c);
	return;
	}

    if(pstat[CD] == HP){
	if((pstat[CM] & 1) != 0){
		plotp(pstat,'}'); plotp(pstat,'~'); plotp(pstat,'\'');
		pstat[CM] =& 0177774;
		}
	goto plotclop;
	}
}
