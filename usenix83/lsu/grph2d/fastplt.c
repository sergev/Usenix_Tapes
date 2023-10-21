#
#include <stdio.h>
#include "nuplot.h"
FILE *fda, *fopen();
FILE *fdb, *fopen();
FILE *fdc, *fopen();
char plotbuf[YSIZE+1][XSIZE+1];
char ys[12];
char xs[12];
int max[YSIZE+1];
int nextfile 0;
int fp;
int ecnt 0;
main()
{
	int y,x;
	fda=fopen("/tmp/plt_data_a","r");
	fdb=fopen("/tmp/plt_data_b","r");
	fdc=fopen("/tmp/plt_data_c","r");
	for (y = 0; y < YSIZE; y++)
	{
		max[y]=0;
		for (x = 0; x < XSIZE; x++)
			plotbuf[y][x] = 0100;
	}
	for (nextfile = 0;nextfile < NPW;nextfile++)
	{
		switch (nextfile)
		{
			case 0:
				fp = fda;
				break;
			case 1:
				fp = fdb;
				break;
			case 2:
				fp = fdc;
				break;
			default:
				printf("Fatal fastplt error\n");
				exit(0);
				break;
		}

		do
		{
			fscanf(fp,"%s%s",&ys,&xs);
			y=atoi(ys);
			x=atoi(xs);
			if (y < 0) dumpbuf();
			else plot(y-nextfile*YSIZE,x);
		}
		while (y != -1 && x != -1);
	}
	if (ecnt>0)
		printf("Fastplt error: %5d points out of bounds",ecnt);
	fclose(fda);
	fclose(fdb);
	fclose(fdc);
}

plot(y,x)
int y,x;
{
	register xx, yy, a;
	a = x / 6;
	if (a > XSIZE-3 || x<0) 
		ecnt++;
	else
	{
		plotbuf[y][a] |= (1 << (x % 6));
		if (a>max[y]) max[y]=a;
	}
}
dumpbuf()
{
	register y,xx;
	for (y=0;y<YSIZE;y++)
	{
		plotbuf[y][max[y]+1] = 05;
		plotbuf[y][max[y]+2] = 012;
		write (1, &plotbuf[y], max[y]+3);
	}
	for (y=0;y<YSIZE;y++)
	{
		for (xx=0;xx<max[y]+3;xx++) plotbuf[y][xx]=0100;
		max[y]=0;
	}
}
