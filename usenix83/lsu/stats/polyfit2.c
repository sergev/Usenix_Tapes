#
#include <stdio.h>
#include <math.h>
#include <h1500.h>
#include <minmax.h>
#define MAXN  500
#define MAXCF	20
FILE *fp, *fopen();
FILE *fq, *fopen();
/************************************************************************
 *									*
 *			POLYFIT.C					*
 *									*
 *	Polynomial curve fitting routine translated from FOCAL		*
 *	Aug 10, 1981, S. Klyce.						*
 *									*
 *	NOTE:  This program contains terminal dependent macros defined	*
 *		in /usr/include/h1500.h (Hazeltine 1500).		*
 *									*
 *		REVISION        HISTORY					*
 *									*
 *	Aug 17, 1981  (klyce).  Added plotting routine to assist in	*
 *				determining best fitting equation.	*
 *									*
 *	Aug 18, 1981  (klyce).	Added check for bug in C lib power	*
 *				function - pow(0,0) gives 0 not 1 as	*
 *				result!					*
 *									*
 *	Aug 31, 1981  (klyce)	Fixed bug in fastscale.c that rescaled	*
 *				data and best fit line individually 	*
 *				which often produced a graphed line	*
 *				not fitting expectation.		*
 *									*
 *	Feb. 1 '82:-  Wm. Marshall					*
 *			Added feature to retain Coefficients of best	*
 *			fit, and to output instantaneous Slope and	*
 *			y-value for and entered x.  The program now	*
 *			Plots only the Bestfit curve (i.e. that with	*
 *			the minimum meansquare deviation).		*
 *									*
 *	Jun 17, 1982 (klyce).  Increased MAXN to 500, and increased	*
 *				the size of various arrays to handle	*
 *				larger (up to 500) number of points.	*
 *									*
 ************************************************************************/
float xmin = HUGE;
float xmax = -HUGE;
float ymin = HUGE;
float ymax = -HUGE;
int numpoints = 500;
int best = 1;
int bestl;
double coef[MAXCF][MAXCF], bestcoef[MAXCF];
double dev[MAXCF];
main()
{
	double x[MAXN],y[MAXN],sx[MAXN],yx[MAXN],b[MAXN],a[MAXN];
	int fits=0;
	int na,nb,l1,j1,l,n2,ii,k,m,mm,kk,n,j,j2;
	double d,dd,sd,z;
	float xplot;
	char *ans;
	int pflag;
	CLS;
	printf("\n\t\tPOLYNOMIAL CURVE FITTER\n\n");
	DIM;
	printf("Do you want data plotted (no) =? ");
	BRIT;
	j=0;
	while ((ans[j]=getchar()) != '\n') j++;
	if (ans[0] == '\n' || *ans == 'n') pflag = 0;
	else pflag = 1;
	DIM;
	printf("Enter lowest equation order =? ");
	BRIT;
	scanf("%d",&na);
	if (na<2)
	{
		sigerror();
		if (na<=0) printf("Dumb!  ");
		if (na==1) printf("We're not into inequalities.  ");
		printf("Lowest equation order adjusted to 2.\n\n");
		na = 2;
	}
	DIM;
	printf("Enter highest equation order =? ");
	BRIT;
	scanf("%d",&nb);
	DIM;
	if (nb<na)
	{
		sigerror();
		printf("Try a little harder.  The highest order can't be less than the lowest order!\n\n");
		nb=na;
	}
	printf("How many data pairs =? ");
	BRIT;
	scanf("%d",&l1);
	if (nb>=l1)
	{
		nb = l1-1;
		sigerror();
		printf("\nThe highest equation order has been reduced to%3d.\n",nb);
		printf("The number of data points fitted must be greater than");
		printf("\nthe number of coeficients.\n\n");
	}
	for (j1=1;j1<=l1;j1++)
	{
		DIM;
		printf("x[%3d]=? ",j1);
		BRIT;
		scanf("%f",&x[j1]);
		DIM;
		printf("y[%3d]=? ",j1);
		BRIT;
		scanf("%f",&y[j1]);
		xmin=min(xmin,x[j1]);
		xmax=max(xmax,x[j1]);
		ymin=min(ymin,y[j1]);
		ymax=max(ymax,y[j1]);
	}
	DIM;
	printf("\n\nDATA POINTS:\n\n No.           X              Y\n");
	BRIT;
	for (j1=1;j1<=l1;j1++)
		printf("%3d       %10.04f    %10.04f\n",j1,x[j1],y[j1]);
	for (l=na;l<=nb;l++)
	{
		n2=2*l-1;
		for (j1=1;j1<=n2;j1++) sx[j1]=0;
		for (j1=1;j1<=l;j1++) yx[j1]=0;
		for (j1=1;j1<=l1;j1++)
			if (x[j1] == 0)
			{
				sx[1]=1;
				yx[1]=y[j1];
			}
		for (j2=1;j2<=n2;j2++)
			for (j1=1;j1<=l1;j1++)
			{
				z=j2-1;
				if (z==0) sx[j2] += 1;
				else sx[j2] += pow(x[j1],z);
			}
		for (j2=1;j2<=l;j2++)
			for (j1=1;j1<=l1;j1++)
			{
				z=j2-1;
				if (z==0) yx[j2] += y[j1];
				else yx[j2] += y[j1]*pow(x[j1],z);
			}
		printf("\n\nNo. of adjustable coefficients = %2d\n",l);
		for (j2=1;j2<=l;j2++)
			for(j1=1;j1<=l;j1++) a[j2+j1*l]=sx[j1+(j2-1)];
		mm=l-1;
		for (ii=1;ii<=mm;ii++)
			for (k=ii;k<=mm;k++)
			{
				n=k+1;
				dd=a[n+ii*l]/a[ii+ii*l];
				for (j=ii;j<=l;j++) a[n+j*l] -= a[ii+j*l]*dd;
				yx[n] -= yx[ii]*dd;
			}
		b[l]=yx[l]/a[l+l*l];
		for (m=2;m<=l;m++)
		{
			n=l+1-m;
			kk=n+1;
			b[n]=yx[n]/a[n+n*l];
			for (k=kk;k<=l;k++) b[n] -= a[n+k*l]*b[k]/a[n+n*l];
		}
		for (k=1;k<=l;k++)
		{
			if (b[k]>1e20 || b[k]<-1e20)
			{
				sigerror();
				printf("BOMBOUT! Overflow in calculation.\nBye bye...\n");
				exit(0);
			}
			else printf("\nb[%2d] =  %g",k,b[k]);
		}
		for (j1=1;j1<=l1;j1++) yx[j1]=0;

		sd=0;
/*
		printf("\n\nNo.     Y Obs           Y Calc            Dif\n");
*/
		if (pflag && !fits) fp = fopen("/tmp/polydata_a","w");
		for (j1=1;j1<=l1;j1++)
		{
			yx[0]=0;
			for (j2=1;j2<=l;j2++)
			{
				z=j2-1;
				if (z==0) yx[0] += b[j2];
				else yx[0] += b[j2]*pow(x[j1],z);
			}
			d=y[j1]-yx[0];
			sd += d*d;
/*
			printf("%2d    %10.04f    %10.04f    %10.04f\n",j1,y[j1],yx[0],d);
*/
			if (pflag && !fits)
				fprintf(fp,"%10.04f %10.04f\n",y[j1],x[j1]);
		}
		if (pflag && !fits) fclose(fp);
		sd = sqrt(sd/(l1-l));
		printf("\nMean square deviation = %10.04f\n",sd);
		fits++;
		dev[fits] = sd;
		for(j=1;j<=l1;j++) coef[fits][j] = b[j];
		bestfit(l,fits);
		for(j=0;j<l1;j++) bestcoef[j] = coef[best][j];
	}
	slopecalc(fits);
	if (pflag)
	{
		fq = fopen("/tmp/polydata_b","w");
		for (j1=0;j1<=numpoints;j1++)
		{
			xplot = xmin + j1*(xmax-xmin)/numpoints;
			yx[0]=0;
			for (j2=1;j2<=l;j2++)
			{
				z=j2-1;
				if (z==0) yx[0] += b[j2];
				else yx[0] += b[j2]*pow(xplot,z);
			}
			fprintf(fq,"%10.04f %10.04f\n",yx[0],xplot);
		}
		fclose(fq);
		printf("OK, enter plot labels now:\n\n");
		system("plotlb2 /tmp/polydata_a -b /tmp/polydata_b -p");
	}
	printf("\nMore polyfits (yes) =? ");
	scanf("%s",ans);
	if (*ans=='\n' || *ans=='y')
	{
		best = 1;
		for(j=0;j<20;j++)
		{
			bestcoef[j] = 0;
			for(kk=0;kk<20;kk++) coef[kk][j] = 0;
		}
		main();
	}
}
sigerror()
{
	int j;
	BELL;
	for (j=0;j<10;j++)
	{
		printf("ERROR\b\b\b\b\b");
		DIM;
		printf("ERROR\b\b\b\b\b");
		BRIT;
	}
	DELL;
}
slopecalc(fits)
int fits;
{
	double x,slope,y;
	double z, zz;
	int i,j;
	char ans;
	printf("\n\n\tBESTFIT COEFFICIENTS\n");
	for(i=1;i<=bestl;i++)
	printf("\n\t[%d] = %10.04f",i,bestcoef[i]);
	if(fits > 1)
	{
		DIM;
		printf("\n\nEnter x value to get y value and inst. slope (no) =? ");
		BRIT;
		ans = getchar();
		ans = getchar();
		while(ans != '\n')
		{
			printf("\nx value = ");
			scanf("%f", &x);
			for(j=2;j<=bestl;j++)
			{
				zz = j-2;
				if(zz == 0.0) slope += bestcoef[j];
				else slope += bestcoef[j]*(j-1)*pow(x,zz);
			}
			for(j=1;j<=bestl;j++)
			{
				zz = j-1;
				if(zz == 0.0) y += bestcoef[j];
				else y += bestcoef[j]*pow(x,zz);
			}
			printf("\nFor x=%10.06f , y=%10.06f and slope = %10.06f\n",x,y,slope);
			DIM;
			printf("More (yes) =? ");
			BRIT;
			ans = getchar();
			if((ans = getchar()) != '\n') break;
			y=x=slope=0.0;
			ans = '0';
		}
	}
}
bestfit(l,fits)	/*Determines best set of coefs for polyfit & retains # coefs.*/
int l;
int fits;
{
	register i;
	for(i=1;i<fits;i++)
	{
		if(dev[i+1] < dev[best])
		{
			best = i+1;
			bestl = l;
		}
	}
}
