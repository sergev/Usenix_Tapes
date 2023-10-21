#include "/v/wa1yyn/c/plot/plot.h"
plotend(pstat)
	int pstat[];
{
    if(pstat[CD] == HP){
	plotpen(pstat,'0');
	plotp(pstat,3);
	write(pstat[FD],pstat[HPBS],pstat[HPBC]);
	flush();
	close(pstat[FD]);
	close(pstat[FDI]);
	}
}
