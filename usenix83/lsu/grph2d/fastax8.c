#
#include <stdio.h>
#include "nuplot.h"
FILE *fp, *fopen();
FILE *fda, *fopen();
FILE *fdb, *fopen();
FILE *fdc, *fopen();

main()
{
	fda=fopen("/tmp/ax8xa","w");
	fdb=fopen("/tmp/ax8xb","w");
	fdc=fopen("/tmp/ax8xc","w");
	praxcrds(0,2,1,0,W,1);
	praxcrds(H-2,H,1,0,W,1);
	praxcrds(2,H-2,1,0,2,1);
	praxcrds(2,H-2,1,W-2,W,1);
	praxcrds(2,9,1,W/10,W,W/10);
	praxcrds(2,9,1,1+W/10,W,W/10);
	praxcrds(H-8,H-2,1,W/10,W,W/10);
	praxcrds(H-8,H-2,1,1+W/10,W,W/10);
	praxcrds(H/10,H,H/10,2,9,1);
	praxcrds(1+H/10,H,H/10,2,9,1);
	praxcrds(H/10,H,H/10,W-8,W-2,1);
	praxcrds(1+H/10,H,H/10,W-8,W-2,1);
	fprintf(fda,"-1 -1\n");
	fprintf(fdb,"-1 -1\n");
	fprintf(fdc,"-1 -1\n");
	fclose(fda);
	fclose(fdb);
	fclose(fdc);
}

praxcrds(ylo,yhi,yinc,xlo,xhi,xinc)
int ylo,yhi,yinc,xlo,xhi,xinc;
{
	int x;
	int y;
	for (y=ylo;y<yhi;y=+yinc)
		for (x=xlo;x<xhi;x=+xinc)
			openwindow(y,x);
}

openwindow(y,x)
int y,x;
{
	y = H - y;
	y += TMGN;
	x += LMGN;
	if (y >= 0 && y <= YSIZE*NPW && x >= 0 && x <= XSIZE*6)
	{
		if (y < YSIZE) fp = fda;
		else if (y < 2*YSIZE) fp = fdb;
		else fp = fdc;
		fprintf(fp,"%4d %4d\n",y,x);
	}
}
