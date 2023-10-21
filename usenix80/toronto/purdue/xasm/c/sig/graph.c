#include	"/v/wa1yyn/c/plot/devsizes"
graph(pstat,lx,x,ymin,ymax,dev,gno)
	int lx,dev,gno,*pstat;	float x[],ymin,ymax;
{
	register i,j,k;
	float	t[1025],scale;	int horz;

	if(dev == 3){
		squeeze(lx,x,TTYX,t);
		horz = TTYX;	scale = TTYY/ymax;
		}
	    else {
		squeeze(lx,x,TEKX,t);
		horz = TEKX;	scale = TEKY/ymax;
		}
	for(i = 0;i != horz;i++)t[i] = (t[i] - ymin)  * scale * 10.0;

	setvec(pstat,'`');
	if(dev != 3){
		i = t[0] + 120;
		plotpen(pstat,gno + '0');
		move(pstat,520,i);
		for(i = 1;i != horz;i++){
			j = t[i] + 120;
			k = i * 10;
			draw(pstat,520 + k,j);
			}
		}
	    else {
		k = 7;
		for(i = 1;i != horz;i++){
			j = t[i];
			lsimov(k++,21-j);
			putchar(gno + '0');
			}
		}
}
