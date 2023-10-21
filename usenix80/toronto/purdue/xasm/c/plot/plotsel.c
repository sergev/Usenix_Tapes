#include "/v/wa1yyn/c/plot/plot.h"
plotsel(pstat,device)
	int pstat[],device;
/*
 *	Select the current plotting device.
 */
{
	register i,j;

	if(device == TEK){
		pstat[CD] = TEK;
		return;
		}
	if(device == HP){
		pstat[CD] = HP;
		return;
		}
	printf("Bad plot device - %d.\n",device);
}
