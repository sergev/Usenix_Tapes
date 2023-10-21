#include "/v/wa1yyn/c/plot/plot.h"

grapha(pstat,title,xmin,xmax,ymin,ymax,dev)
	int dev,*pstat;	char *title;	float xmin,xmax,ymin,ymax;
{
	register i,j,k;	float x;
	char foo[30];

	if(dev == 1){
		plotsel(pstat,TEK);
		}
	if(dev == 2){
		plotsel(pstat,HP);
		}
	if(dev != 3){plotinit(pstat);
		}

	if(dev == 3){				/* to tty */
		putchar('Z' & 037);	putchar(1);	putchar(1);
		lsimov(1,1);
		printf("\t\t\t%s",title);
		i = 1;
		x = (ymax-ymin)/5.0;
		for(j = 1;j != 7;j++){
			lsimov(1,i);
			printf("%5.2f",((6-j) * x)+ymin);
			i =+ 4;
			}
		lsimov(6,22);	x = (xmax - xmin) / 5.0;
		for(j = 0;j != 6;j++){
			printf("%5.2f",j * x);
			printf("        ");
			}
		for(i = 1;i != 22;i++){
			lsimov(7,i);	putchar('|');
			}
		lsimov(7,21);	putchar('L');
		for(i = 8;i != 76;i++){
			lsimov(i,21);	putchar('_');
			}
		return;
		}
	    else {
		move(pstat,4000,7000);	goalpha(pstat);
		setchr(pstat,'4');
		plots(pstat,title);
		setchr(pstat,'1');
		move(pstat,520,120);	draw(pstat,520,7000);
		move(pstat,520,120);	draw(pstat,10200,120);
		i = 120;	x = (ymax-ymin) / 5.0;
		for(j = 0;j != 6;j++){
			move(pstat,20,i);	goalpha(pstat);
			sprintf(foo,"%5.2f",j * x);
			plots(pstat,foo);
			i =+ 1200;
			}
		i = 440;	x = (xmax - xmin) / 5.0;
		for(j = 0;j != 6;j++){
			move(pstat,i,20);	goalpha(pstat);
			sprintf(foo,"%5.2f",j * x);
			plots(pstat,foo);
			i =+ 1600;
			}
		}
}

