/*
	This program precesses coordinates from one equinox to another.
	Uses standard input and output.

	usage: precess [final equinox]
	input: star_name h m s +/-d m s start_equinox [final_equinox]

	The final equinox must be specified upon execution or within the
	standard input. You MUST include the sign of the declination.

	Example:
		NGC1333 03 27 36 +31 17 00 1975 1950
	will precess NGC1333 to 1950. 

	Compile: cc -o precess precess.c -lm

	Author: Sean Casey Feb 24 1986 ( Original version in Vax Fortran )

*/
#include <stdio.h>
#include <math.h>
#define RAD 206264.8062
#define RAD15 (RAD/15.0)

main (argc,argv)
int argc;
char *argv[];
{
    double eqobs, equn1, equn0, a,d,rh,rm,rs,dg,dm,ds;
    double deq,deq2,t1900,pm,pn,pam,pdm,am,pa,pd,ap,dp;
    double atof();
    
    int cnt, hr,min,sec;
    char object[80], sign,input[80];


    fprintf(stderr,"%s Ver 1.0 by S. Casey Feb 24, 1986\n",argv[0]);
    
    if (argc == 2) {
        eqobs = atof(argv[1]);
    }
    
    while (gets(input) != NULL) {
	cnt=sscanf(input,"%s %lf %lf %lf %c %lf %lf %lf %lf %lf",object,&rh,&rm,&rs,&sign,&dg,&dm,&ds,&equn0,&equn1);
	if (cnt == 10 || cnt == 9) { /* if this looks valid, do it */
            if ((sign != '+' && sign != '-') || (cnt < 10 && argc != 2)) {
		fprintf(stderr,"%s:Invalid input\n",argv[0]);
		exit(1);
	    }
	    if(cnt == 9 && argc == 2)equn1=eqobs;
	
	    a = (rh*3600.0 + rm*60.0 + rs)/RAD15;
	    d = (dg*3600.0 + dm*60.0 + ds)/RAD;
	    if(sign == '-')d = -d;

	    /* precess to final equinox using newcomb precession */
	    deq = equn1-equn0;
	    deq2 = deq/2.0;
	    t1900 = equn1-deq2-1900.0;
	    pm=46.0850+0.000279*t1900;
	    pn=20.0468-0.000085*t1900;
	    pam=pm+pn*sin(a)*sin(d)/cos(d);
	    pdm=pn*cos(a);
	    am=a+pam*deq2/RAD;
	    dm=d+pdm*deq2/RAD;
	    pa=pm+pn*sin(am)*sin(dm)/cos(dm);
	    pd=pn*cos(am);
	    am=a+pa*deq2/RAD;
	    dm=d+pd*deq2/RAD;
	    pa=pm+pn*sin(am)*sin(dm)/cos(dm);
	    pd=pn*cos(am);

	    ap=(a*RAD15+pa*deq/15.0)/3600.0;
	    dp=(d*RAD+pd*deq)/3600.0;
	    if(ap<0.0)ap +=24.0;
	    sign = '+';
	    if(dp<0.0) {
	        dp = -dp;
	        sign = '-';
	    }

	    /* print out the data */	
	    printf("%s",object);
	    hr = ap;    /* do RA */
	    min = ap*60.0; min = min % 60;
	    sec = ap*360000.0; sec = sec % 6000;
	    printf("\t%2d %2d %5.2f",hr,min,(float)sec/100.0);
	    hr = dp;    /* do DEC */
	    min = dp*60.0; min = min % 60;
	    sec = dp*360000.0; sec = sec % 6000;
	    printf(" %c%2d %2d %5.2f",sign,hr,min,(float)sec/100.0);
	    printf(" %6.1f\n",equn1);
	}
	
    }
    exit(0);
}

