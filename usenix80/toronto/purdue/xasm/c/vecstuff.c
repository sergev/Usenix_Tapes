#include "/v/wa1yyn/c/plot/plot.h"
	int pstat[20];

main(argc,argv)
	int argc;	char **argv;
{
	register x,y;
	register char c;
	int x2,y2;
	int i,j;
	char s[100];
	char plotob[600];

printf("plotob is '%o'.\n",plotob);
	pstat[HPBS] = plotob;
	j = '`';
	pstat[DEBUG] = 1;
	plotsel(pstat,HP);
	plotinit(pstat);
	pstat[DEBUG] = 0;

    while(1){
	scanin(s);
	x = basin(s,10);
	scanin(s);
	y = basin(s,10);
	scanin(s);
	x2 = basin(s,10);
	if(scanin(s) == -2)return;
	y2 = basin(s,10);
	move(pstat,x,y);
	draw(pstat,x2,y2);	
	mover(pstat,-1000,-1000);	
	write(pstat[FD],pstat[HPBS],pstat[HPBC]);
	pstat[HPBC] = 0;
	pstat[HPBP] = pstat[HPBS];
	}


}
