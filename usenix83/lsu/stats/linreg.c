/*program to calculate slope, y intercept, correlation coefficient, and 95% confidence limits*/
/*for linear regression*/
/*Karen P. Mann*/
/*July 11, 1980*/
/************************************************************************/
/*									*/
/*			REVISION HISTORY 				*/
/*									*/
/*  Updated Sept 9/80 to include angular transform of % data  WSM       */
/*                                                                      */
/*  Oct. 15, 1980.  Corrected errors in calculation of conf. ints. WSM  */
/*                                                                      */
/*  Oct. 20, 1980.  Corrected error in calc of r and added patch to     */
/*  output significance of r.                   (s. klyce)              */
/*                                                                      */
/*  Oct. 23, 1980.  Added patches to save or plot linear regression     */
/*  data, confidence interval, and data points on the line printer.     */
/*  Bugs:  user must make sure the system load is low when using the    */
/*  plotter or the system will run out of putc buffers...HELP, jon day! */
/*                                                                      */
/*                                             (s. klyce)               */
/*                                                                      */
/*  Nov. 27, 1980.  Changed input to read in coordinates as x,y pairs.  */
/*									*/
/*				               (s. klyce)   		*/
/*  Dec. 3, l980.  Added error  & 95% confidence interval of slope      */
/*	           and added t calculation for two data sets entered    */
/*                 in sequence, to determine if slopes are different.   */
/*                  ...also cleaned up output table...                  */
/*                                              (w. marshall)           */
/*                                                                      */
/*	Removed system status output, since inode and buffer space      */
/*  increased.  Increased the number of coordinates plotted for         */
/*  linear fit and confidence interval.  Changed plot character         */
/*  to a single point to ease confusion on output.                      */
/*									*/
/*						(s. klyce)		*/
/*	Oct 12/81.	Changed plot to plotlb2, where the bestfit	*/
/*	line & 95% conf. interval plotted as points & the points	*/
/*	are plotted as boxes.  Now output graphics bear axis labels.	*/
/*						(w. marshall)		*/
/*	Feb  2, 1982.  Changed order of y and x in /tmp/points to	*/
/*	conform to local plotting s'ware.				*/
/*		The convention for any to be plotted file is:		*/
/*									*/
/*			y0	x0					*/
/*			y1	x1					*/
/*			:	:					*/
/*			:	:					*/
/*			yn	xn					*/
/*									*/
/*						(klyce)			*/
/*									*/
/*									*/
/*  Mar.  2, 1983.  Switched format specifiers in scanf calls from %f	*/
/*  to %lf to eliminate occasional errors on multiple runs (ensures	*/
/*  low bytes of double variables are rezeroed.				*/
/*									*/
/*						(s klyce)		*/
/*									*/
/************************************************************************/
#
#include <math.h>
#include <stdio.h>
FILE *fp, *fopen();
FILE *fq, *fopen();
char ans;
char anss;
int i, j, n, m, o, ko;
/* Variables for angular transformation of % to degrees */
int bflag={1}, b1flag={0};
int transflag={0};
double asin();
double sqrt();
double corterm,errvar,dev;
double z, berr;
double b1, sumsqx1, tb;
int runflag={1};
double a, b, coef, be, re, sumx, sumy, t, xlower, xupper, sumxy, sumysq, sumxsq;
double x[1000]; 
double y[1000];
double c[1000];
int num_pnts = 300;
main()
{
	printf("\n\n\n\tLINEAR REGRESSION WITH 95%% CONFIDENCE LIMITS");
	printf("\n\n\tDO YOU HAVE PERCENTAGE DATA ? (y or n) : ");
	scanf("%s", &ans);
	if(ans == 'y')
		{
		printf("\nWe recommend an angular transform of the y-axis\n");
		printf("from 0-100%% to 0-90 degrees.  This will transform\n");
		printf("a sigmoidal log dose-response curve to a straight\n");
		printf("line and improve the regression.  Do you want it? :");
		scanf("%s", &ans);
		if(ans == 'y') 
			{
			transflag = 1;
			}
		}
	printf("\n\nEnter data as pairs, x coordinate first.  After final x coordinate type '-1e10'\n\n");
	for (i=0; i<20; i++) printf(" ");
	for (i=0; i<8; i++) printf("<> ");
	printf("\n\n");
	while (runflag)
	{
		n=0;
		/*input x,y coordinates*/
		while (1)
		{
			printf("x[%3d]= ",n+1);
			scanf("%lf",&x[n]);
			if (x[n] == -1e10) break;
			printf("y[%3d]= ",n+1);
			scanf("%lf",&y[n]);
			n++;
		}
		matprint();
		loop:
		printf("\nAny corrections? (y or n) ");
		scanf("%s",&ans);
		anss=ans;
		if (ans == 'y') correct();
		else if (ans != 'n') goto loop;
		if (anss=='y') matprint();
		printf("Enter t value for %3d D.F. at the 95% confidence level\n",n-2);
		scanf("%lf",&t);
		printf("t= %f\n",t);
		loopb:
		printf("Any corrections? (y or n) ");
		scanf("%s",&ans);
		if (ans == 'y') acorrect();
		else if (ans != 'n') goto loopb;
		if(transflag) transform();
		regrcalc();
		printf("\n\nMore data? (y or n) ");
		scanf("%s",&ans);
		if (ans=='n') runflag=0;
	}
}


regrcalc()
{
	double g;
	float convert();
	sumx=sumy=sumxsq=sumysq=sumxy=0;
	z = n;
	/*calculate the sum of the x's(sumx), the sum of the y's(sumy),*/
	/*the sum of the squares of the x's and y's(sumxsq and sumysq),*/
	/*and the sum of the products of the x's and y's(sumxy)*/
	for (i=0; i<n; i++)
	{
		sumx = sumx + x[i];
		sumy = sumy + y[i];
		sumxsq = sumxsq + x[i]*x[i];
		sumysq = sumysq + y[i]*y[i];
		sumxy = sumxy + x[i]*y[i];
		c[i] = x[i];
	}
	/*calculate the correlation coefficient(coef), the y intercept(a) and the slope(b)*/
	a = (sumy * sumxsq - sumx * sumxy)/(n * sumxsq - sumx * sumx);
	b = (n * sumxy - sumx * sumy)/(n * sumxsq - sumx * sumx);
	coef=(sumxy-sumx*sumy/z)*(sumxy-sumx*sumy/z)/((sumxsq-sumx*sumx/z)*
		(sumysq-sumy*sumy/z));
	coef=sqrt(coef);

	/*sorter*/
	/*find the lowest and highest values of x: xlower & xupper*/
	m = n;
	for (j=1; j<=n-1; j++)
	{
		m=m-1;
		for (i=0; i<m; i++)
		{
			if ((c[i+1]-c[i])<0)
			{ 
				re = c[i];
				c[i] = c[i+1];
				c[i+1] = re;
			}
		}
	}
	xlower = c[0];
	xupper = c[n-1];
	for(i=0 ; i<n ; i++) corterm =+ (x[i]-sumx/z)*(x[i]-sumx/z);
	errvar = (sumysq - sumy*sumy/z - b*(sumxy - sumx*sumy/z))/(z-2);
	berr =  (t*sqrt(errvar))/(sqrt(sumxsq-(sumx*sumx)/z));
	printf("\n\n");
	for(i=0 ; i<65 ; i++) printf("=");
	printf("\n\n  PARAMETER\t\t\t\t95%% CONFIDENCE INTERVAL\n");
	for(i=0 ; i<65 ; i++) printf("=");
	printf("\nSlope       = %f +/- %f \t(%f to  %f)", b, berr, b-berr, b+berr);
	dev = t*sqrt(errvar*(1/z + (sumx/z*sumx/z)/corterm));
	printf("\ny-intercept = %f +/- %f \t(%f to  %f)", a, dev, a-dev, a+dev);
	printf("\nCorrelation\ncoefficient = %10.05f\n",coef);
	printf("Significance of correlation coefficient: t = %10.05f\n",sqrt(coef*coef*(n-2)/(1-coef*coef)));
	bcomp(); b1flag = 1;
	for(i=0 ; i<65 ; i++) printf("=");
	printf("\n\n");
	printf("Value of X     Y estimate      + Y dev     - Y dev\n");
	g = xlower;
	for(i=0 ; i<=20 ; i++)
	{
		dev = t*sqrt(errvar*(1/z + (g-sumx/z)*(g-sumx/z)/corterm));
		printf("%10.05f    %10.05f\n", g, convert(b*g+a));
		printf("%10.05f                %10.05f\n", g, convert(b*g+a+dev));
		printf("%10.05f                         %10.05f\n",g,convert(b*g+a-dev));
		g = g+((xupper-xlower)/20);
	}
	if (transflag) restor();	/* restor y vals to original % data */
	for(i=0 ; i<n ; i++)
	{
		printf("%10.05f\t%10.05f\n", x[i], y[i]);
	}
	lp:
	printf("\n\nDo you want to save the data ? (y or n) ");
	scanf("%s",&ans);
	if (ans=='y') plot();
	else if (ans != 'n') goto lp;
}

/*print out x and y coordinates*/
matprint()
{
	int ko;
	ko=0;
	while (ko < n)
	{
		for (i=0; i<5; i++)
		{
			if (i+ko < n)
			{
				printf(" x[%3d]= %6.02f",ko+1+i,x[ko+i]);
				printf("    y[%3d]= %6.02f\n",ko+1+i,y[ko+i]);
			}
		}
		ko =+ 5;
		printf("\n");
	}
	printf("\n\n");
}


/*correct x and y coordinates*/
correct()
{
	loopc:
	printf("\nEnter index no. of wrong entry =? ");
	scanf("%d",&i);
	printf("\nWhich is coordinate? (x or y) ");
	scanf("%s",&ans);
	if (ans == 'x')
	{
		printf("\nx[%3d]=? ",i);
		scanf("%lf",&x[i-1]);
	}
	else if (ans == 'y')
	{
		printf("\ny[%3d]=? ",i);
		scanf("%lf",&y[i-1]);
	}
	else goto loopc;
	loopd:
	printf("More ? (y or n)");
	scanf("%s",&ans);
	if (ans == 'y') goto loopc;
	else if (ans != 'n') goto loopd;
}

/*correct t value*/
acorrect()
{
	loope:
	printf("T value?\n");
	scanf("%lf",&t);
	printf("T= %f\n",t);
	loopf:
	printf("Any corrections? (y or n) ");
	scanf("%s",&ans);
	if(ans == 'y') goto loope;
	else if (ans != 'n') goto loopf;
}
transform()
{
	printf("\n\n\nTRANSFORMED Y VALUES:\n\n");
	for(i=0 ; i<n ; i++)
	{
	y[i] = asin(sqrt(y[i]/100.));   /* ANGULAR TRANSFORM */
	y[i] = y[i]*180./PI;   /* RADIANS TO DEGREES */
	printf("  y[%d] = %6.3f degrees\n", i+1, y[i]);
	}
}
restor()
{
	for(i=0;i<n;i++) {
		y[i] /= (180./PI);
		y[i] = sin(y[i]);
		y[i] *= y[i]*100.;
	}
}
float convert(y)
float y;
{
	if (transflag) {
		y /= (180./PI);
		y = sin((double)y);
		y *= y*100.;
	}
	return(y);
}
plot()
{
	double g;
	fp = fopen("/tmp/lrdata","w");
	fq = fopen("/tmp/points", "w");
	g = xlower;
	for(i=0 ; i<=num_pnts ; i++)
	{
		dev = t*sqrt(errvar*(1/z + (g-sumx/z)*(g-sumx/z)/corterm));
		fprintf(fp,"%10.05f    %10.05f\n", convert(b*g+a),g);
		fprintf(fp,"%10.05f                %10.05f\n", convert(b*g+a+dev),g);
		fprintf(fp,"%10.05f                         %10.05f\n",convert(b*g+a-dev),g);
		g = g+((xupper-xlower)/num_pnts);
	}
	for(i=0 ; i<n ; i++)
	{
		fprintf(fq,"%10.05f\t%10.05f\n", y[i], x[i]);
	}
	fclose(fp);
	fclose(fq);
	loop:
	printf("Do you want a plot (p) or save data (s) ? ");
	scanf("%s",&ans);
	if (ans=='p')
	{
		printf("Data being plotted; wait to enter labels and title...\n");
		system("plotlb2 /tmp/points -b /tmp/lrdata -p");
	}
	else if (ans=='s')
	{
		system("mv /tmp/lrdata $s/lrdata$$$b");
		system("= b $ba");
		system("echo your data is saved in $s/lrdata");
	}
	else goto loop;
}
bcomp()
{
	if(bflag)
	{
	b1 = b;
	sumsqx1 = sumxsq - sumx*sumx/z;
	bflag=0;
	}
	if(b1flag)
	{
	tb = (b-b1)/(sqrt(errvar)*sqrt(1/((sumxsq-(sumx*sumx)/z)+(1/sumsqx1))));
	if(tb < 0.0) tb = -tb;
	printf("\n\nSlope comparison:\n\t\tthis regression: %f\n\t\tlast regression: %f\n\t\tt = %f\n", b, b1, tb);
	b1 = b;
	sumsqx1 = sumxsq - sumx*sumx/z;
	}
}
