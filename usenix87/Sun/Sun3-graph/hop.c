
/*
 * hop.c adapted from scientific american, 9/86, p.14+
 *
 * version for Sun-3 use.
 *
 * cc -w -O -o hop hop.c -lm -lpixrect
 *
 * arguments: a b c magx center limit
 *   a, b, c are parameters of the equation
 *   magx (==magy) allows blowing up the image to reveal fine structure
 *   center allows the image to be shifted on the screen 
 *      (useful when magnifying)
 *   limit is number of points to plot
 *
 *   all arguments have defaults
 *
 * note the defines for color screens.
 * you may have to change the dev name in the open statement.
 *
*/

#include <stdio.h>
#include <pixrect/pixrect_hs.h>

#define MARGIN 3 /* offset from side of sun screen */
#define COLOR 1

/* for color screens... this colors a point according to the number
   of iterations it takes to get to the point in the sequence. 
   the multiplier is to slow down the color changes, and the mod'ing
   is to keep it in the palette

#define NUMCOLORS 256
#define DELTACOLOR 10
#define COLOR ((((k/NUMCOLORS)+1)*DELTACOLOR)%(limit/NUMCOLORS))
*/

main(argc,argv)
int argc;
char **argv;
{

	double atof(), sqrt(), fabs();
	double i,j,inew,jnew;
	double a = 5.0;
	double b = 13.0;
	double c = 1400.0;
	int k, limit=100000, center=400, magx=1, magy=1;
	struct pixrect *pr;
	FILE *fp;

	if (argc > 1) {
		a = atof(*++argv);
		b = atof(*++argv);
		c = atof(*++argv);
	}
	if (argc > 4) {
		magx = atoi(*++argv);
		magy = magx;
	} 	
	if (argc > 5) {
		center = atoi(*++argv);
	}
	if (argc > 6) {
		limit = atoi(*++argv);
	}

	pr = pr_open("/dev/fb");
	pr_rop(pr,0,0,pr->pr_size.x - 1,pr->pr_size.y - 1, PIX_CLR,NULL,0,0);

        i = j = 0.0;
   	for (k=0; k<limit; k++) {
		pr_put(pr, (int)(i*magx)+center+MARGIN,
		    	   (int)(j*magy)+center+MARGIN, 
			   COLOR ); 
		inew = j - (i<0?-1:1) * sqrt(fabs(b*i - c));
		jnew = a - i;
		i = inew;
		j = jnew;
	}
}
