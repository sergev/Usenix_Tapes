#
#include <stdio.h>
#include <math.h>
#include <minmax.h>
#include "nuplot.h"
#define N 4900
FILE *fp, *fopen();
FILE *fda, *fopen();
FILE *fdb, *fopen();
FILE *fdc, *fopen();
float ni;
float yrange,xrange;
int i;
int rflag;
int noscale 0;
int yy,xx;
int plot2 = 0;
float y[N];
float x[N];
float ymin = {HUGE};
float ymax= {-HUGE};
float xmin = {HUGE};
float xmax = {-HUGE};
float yscale;
float xscale;
int squares = 0;
int xes = 0;
int plus = 0;
int points = 0;
int box = 0;
char *arg1[] = "/tmp/plt_data_a";
char *arg2[] = "/tmp/plt_data_b";
char *arg3[] = "/tmp/plt_data_c";
char *arg4[] = "/tmp/gscale";

main(argc,argv)
int argc;
char *argv[];
{
	int ko;
	int ymini,xmini,r;
	static int ryflag =0;

	if (strcmp(*++argv,"-p\0")==0) points=1;
	else if (strcmp(*argv,"-x\0")==0) xes=1;
	else if (strcmp(*argv,"-+\0")==0) plus=1;
	else if (strcmp(*argv,"-b\0")==0) box=1;
	else if (strcmp(*argv,"-n\0")==0) noscale=1;
	else squares=1;
	if (noscale)
	{
		squares=0;
		points = 1;
	}
	if (strcmp(*++argv,"-2\0")==0)
	{
		plot2=1;
		fp=fopen("/tmp/gscale","r");
		fscanf(fp,"%e%e%e%e%e%e%d",&ymin,&ymax,&xmin,&xmax,&yscale,&xscale,&ryflag);
		fclose(fp);
	}
	for (i=0;i<N;i++)
	{
		scanf("%f%f",&y[i],&x[i]);
		if (y[i]== -1 && x[i]== -1)
		{
			ko=i;
			i=N;
		}
		else if (!plot2 && i<N-1)
		{
			ymin=min(ymin,y[i]);
			ymax=max(ymax,y[i]);
			xmin=min(xmin,x[i]);
			xmax=max(xmax,x[i]);
		}
		else if (i>=N-1)
		{
			ko=i;
			if (!noscale) printf("fastscale: data base too large, ignoring points beyond first 4900 coords...\n");
			else
			{
				writf(ko,0);
				ko=i=0;
			}
		}
	}
	yrange=ymax-ymin;
	xrange=xmax-xmin;
	if (yrange==0 && xrange==0)
	{
		printf("fastscale: data all 0's\n");
		exit();
	}
	if (!plot2 && !noscale)
	{
		r=adjustr(yrange);
		if (rflag) r= -r;
		ymini=ni*ymin*.01;
		if (ymin<0) ymini--;
		ymin=ymini*100/ni;
		yrange=ymax-ymin;
		r=adjustr(yrange);
		if (rflag)
		{
			r= -r;
			ryflag=1;
		}
		ymax=ymin+r/ni;
		r=adjustr(xrange);
		if (rflag) r = -r;
		xmini=ni*xmin*.01;
		if (xmini<0) xmini--;
		xmin=xmini*100/ni;
		xrange=xmax-xmin;
		r=adjustr(xrange);
		if (rflag) r= -r;
		xmax=xmin+r/ni;
		yscale=H/(ymax-ymin);
		xscale=W/(xmax-xmin);
	}
	if (!noscale)
	{
		for (i=0;i<ko;i++)
		{
			y[i]=(y[i]-ymin)*yscale;
			if (ryflag) y[i]= H-y[i];
			x[i]=(x[i]-xmin)*xscale;
		}
	}
	writf(ko,1);
	if (!plot2)
	{
		fp = fopen(*arg4,"w");
		fprintf(fp,"%20.05e%20.05e%20.05e\n",ymin,ymax,xmin);
		fprintf(fp,"%20.05e%20.05e%20.05e %2d\n",xmax,yscale,xscale,ryflag);
		fclose(fp);
	}
}
		

adjustr(range)
float range;
{
	ni = 1;
	rflag=0;
	if (range<0)
	{
		rflag=1;
		range = -range;
	}
	while (range<100)
	{
		range =* 10;
		ni =* 10;
	}
	while (range>1000)
	{
		range =/ 10;
		ni =/ 10;
	}
	if (range<=200) return(200);
	else if (range<=500) return(500);
	else return(1000);
}
drawsq()
{
	register ii,jj;
	for (ii=yy-1;ii<=yy+1;ii++)
		for (jj=xx-1;jj<=xx+1;jj++)
			openwindow(ii,jj);
}
drawx()
{
	openwindow(yy+1,xx-1);
	openwindow(yy+1,xx+1);
	openwindow(yy-1,xx-1);
	openwindow(yy-1,xx+1);
	openwindow(yy,xx);
}
drawpl()
{
	openwindow(yy,xx+1);
	openwindow(yy,xx-1);
	openwindow(yy+1,xx);
	openwindow(yy-1,xx);
	openwindow(yy,xx);
}
drawbox()
{
	register ii;
	for (ii=yy-2;ii<=yy+2;ii++)
	{
		openwindow(ii,xx-2);
		openwindow(ii,xx+2);
	}
	for (ii=xx-2;ii<=xx+2;ii++)
	{
		openwindow(yy+2,ii);
		openwindow(yy-2,ii);
	}
}
putsymbol(i)
int i;
{
	int flag;
	xx=x[i];
	yy=y[i];
	if (yy != (int)y[i-1] || xx != (int)x[i-1]) flag=1;
	else flag=0;
	if (i==0 || flag)
	{
		if (squares) drawsq();
		else if (xes) drawx();
		else if (points) openwindow(yy,xx);
		else if (plus) drawpl();
		else if (box) drawbox();
	}
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

writf(n,term)
int n,term;
{
	register i;
	fda = fopen(*arg1,"a");
	fdb = fopen(*arg2,"a");
	fdc = fopen(*arg3,"a");
	putsymbol(0);
	for (i=1;i<n-1;i++)
	{
		putsymbol(i);
	}
	putsymbol(n-1);
	if (term && noscale)
	{
	}
	fclose(fda);
	fclose(fdb);
	fclose(fdc);
}
