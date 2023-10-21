#include "/v/wa1yyn/c/plot/plot.h"
plotcls(pstat)
	int pstat[];
{
/*	Clear the screen and set to alpha mode,
 *	and go to home postion.
 */
	if(pstat[CD] == TEK){
		plotp(pstat,ESC);	plotp(pstat,FF);
		pstat[CM] = 0;	/* reset graphics, next draw, etc */
		pstat[X] = 0;
		pstat[Y] = 7800-220;
		flush();
		sleep(2);
		}
	if(pstat[CD] == HP){
		move(pstat,0,75800);
		}
}
