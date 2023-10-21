#include "/v/wa1yyn/c/plot/plot.h"
plotp(pstat,data)
	char data;	int pstat[];
{
	register char *p;
	register i,j;
	int flag,tickle,waitc;
	char ansbuff[32];
/*	routine to put an output on the correct plotting device
 *	as determined by the ploting status vector.
 */
    if(pstat[CD] == TEK)
		putchar(data);
	else {
	p = pstat[HPBP];
	*p++ = data;	pstat[HPBP] = p;
	pstat[HPBC]++;
	if(pstat[HPBC] == 512){
		write(pstat[FD],pstat[HPBS],512);
		pstat[HPBC] = 0;
		pstat[HPBP] = pstat[HPBS];
again:		tickle = '\05';
		write(pstat[FD],&tickle,1);	/* send ENQ	*/
		i = read(pstat[FDI],ansbuff,30);
		flag = 0;	p = ansbuff;
		for(j = 0;j < i;j++)if(*p++ == 'G')flag = 1;
		if(flag == 0)goto again;
		}
	}
}
