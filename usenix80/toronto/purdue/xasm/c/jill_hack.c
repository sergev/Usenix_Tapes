/*			EE 301
		    Assignment #4
	
		   Jill J. Hallenbeck
			2/5/79
								*/
/*	This is a program to compute the Convolution of two signals
	The user has the option of plotting the result on the terminal,
	on the HP Plotter or on the Tektronix graphics terminal by
	changing the option given the subroutines.		*/
#define NPNT 256		/* number of time intervals */
#define YMAX 2.0
#define PDEV 3
#include "/v/wa1yyn/c/plot/plot.h"
	extern fout;
	double exp();
	double t;
	int pstat[20];
	char plotob[600];
main(argc,argv)
	int argc;	char **argv;
{
	register i,j,k;
	double r;
	float x[NPNT],y[NPNT],result[NPNT],time;
	int lx,ly,lr;
	double amp,slice,unit,clength;

	fout = dup(1);
	pstat[HPBS] = plotob;

/* Zero out the arrays */

	presto(NPNT,x,0.);	presto(NPNT,y,0.);

/* Describe the triangle fcn */

	lx = 64;
	slice = 2.0/lx;		/* smallest time interval delta t */
	unit = lx/2.0;	/* # of slices in a unit interval */

	k = unit ;		/* k = unit ? */
	for(i = 0;i < k;i++){x[i] = 2.0 * (i * slice); /* rising */
		}
	j = (2 * unit) +1;
	for(;i < j;i++){x[i] = 2.0 - (2.0 * ((i - unit) * slice));
		}


/* Describe the exponential fcn */

	ly = 128;
	slice = 4.0/ly;
	unit = ly/4.0;
	for(i = 0;i <= NPNT;i++){
		t = slice * i;		/* t is time */
		r = 2 * t;
		y[i] = exp(-t) - exp(-r);  /* exponential fcn */
		if(t >= 4.0)y[i] = 0;
		}

/*	Both waveforms are generated	*/

/* convolve the signals */

	fold(lx,x,ly,y,lx + ly - 1,result,NPNT/8.0);

/* print the results of the two fcns and their convolution */

	for(i = 0;i != 200;i++){
		time = i;
		time = time / 32.0;
		printf("X[%5.4f] = %5.4f, Y[%5.4f] = %5.4f, result[%5.4f] = %5.4f\n",time,x[i],time,y[i],time,result[i]);
		}

	flush();

/*	Plot the result			*/

/*	grapha(pstat,"title",xmin,xmax,ymin,ymax,dev)
				dev	1 = TEK  2 = HP  3 = tty
	This just does the lable, must be first called as sets
		up other things.
				*/

	/* graph some axes and the data */

	grapha(pstat,"	Convolution Problem",0.0,6.0,0.0,YMAX,PDEV);
	graph(pstat,192,x,0.0,YMAX,PDEV,1);
	graph(pstat,192,y,0.0,YMAX,PDEV,2);
	graph(pstat,192,result,0.0,YMAX,PDEV,3);


	lsimov(0,22);
	flush();
}
/* fcns 'graph' and 'grapha' are part of the standard 'C' plotting
	routines and can be called by the program.  These are
	part of the Unix system plotting package.		*/
