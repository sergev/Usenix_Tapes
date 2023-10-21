/*  Planets
 *           A program to determine the position of
 *	     the Sun., Mer., Ven. and Mars

 *  reference Jean Meesus's book, " Astronomical Formulae for 
 *  Calculators 
 *			program by     
 *
 *			F.T. Mendenhall  
 *			ihnp4!inuxe!fred
 *
 *		Copyright 1985 by F.T. Mendenhall
 *		all rights reserved
 *
 * (This copyright prevents you from selling the program - the
 *  author grants anyone the right to copy and install the program
 *  on any machine it will run on)
*/

#include <stdio.h>
#include <math.h>
double kepler();
double truean();
double longi();
double lati();
double poly();
double aint();
double range();
int cls();
int trans();
int speak();
double pie,rad;
main()
{
	int mm,dd,yy,time,tz,aa1,bb1;
	double jd,year,month,day,T0;
	double l,a,e,i,w1,w2,M,M0,M1,M2,M4,M5,M6,M7,M8;
	double a0,a1,a2,a3,RA,DEC;
	double ECC,nu,r,u,ll,b,lonpert,radpert;
	double esun,Lsun,Cen,Stheta,Snu,Sr;
	double N,D,epli,thapp,omeg;
	
	cls();
	pie = 3.141592654;
	rad = pie/180.0;
	printf("DATE AND TIME FOR DESIRED PLANETARY POSITION?\n");
	printf("What is the month (1-12)? ");
	scanf("%d",&mm);
	printf("\nWhat is the day (1-31)? ");
	scanf("%d",&dd);
	printf("\nWhat is the year (xxxx)? ");
	scanf("%d",&yy);
	printf("\n\nWhat is the time in the 24 hour clock?");
	printf("\n (note 8:21 AM = 0821; 2:45 PM = 1445) \n\n");
	printf(" RECALL  1pm = 1300   2pm = 1400  3pm = 1500 \n");
	printf("         4pm = 1600   5pm = 1700  6pm = 1800 \n");
	printf("         7pm = 1900   8pm = 2000  9pm = 2100 \n");
	printf("        10pm = 2200  11pm = 2300 12pm = 2400 \n");
	printf("\n What is the time of interest?");
	scanf("%d",&time);
	printf("\n\nWhat is the time zone?");
	printf("\nInput 0 for Universal time -\n");
	printf("Input 1 for EST - \n");
	printf("Input 2 for CST - \n");
	printf("Input 3 for MST - \n");
	printf("Input 4 for PST - \n");
	printf("What is the time zone? ");
	scanf("%d",&tz);
	cls();
	printf("For the date %d/%d/%d\n",mm,dd,yy);
	printf(" At %d ",time);
	if ( tz == 0)
	{
		printf("Universal Time");
	}
	else if ( tz == 1)
	{
		printf("EST");
	}
	else if ( tz == 2 )
	{
		printf("CST");
	}
	else if ( tz == 3 )
	{
		printf("MST");
	}
	else 
	{
		printf("PST");
	}
	printf("\n the Julian date is ");
/*   */
/*  Calculate the Julian date for the time of observation */
	if( tz != 0)
	{
		tz = tz + 4;
	}
	
	day = ((float)dd + ((float)time +(float)tz*100.0)/2400.0);
	if (mm > 2)
	{
		year = yy;
		month = mm;
	}
	else
	{
		year = yy - 1;
		month = mm + 12;
	}
	aa1 = (int) (year/100);
	bb1 = 2 - aa1 + (int)(aa1/4);
	jd = aint(365.25*year) + aint(30.6001*(month + 1));
	jd = jd  + day + 1720994.5;
	if (year < 0)
	{
		cls();
		printf("Negative year not allowed- abort");
		exit(1);
	}
	if((year + month/100) > 1582.10)
	{
		jd = jd + bb1;
	}
	printf("%f\n",jd);
	
/*  calculate time T0 from 1900 January 0.5 ET */

	T0 = (jd - 2415020.0)/36525.0;

/* Calculate orbital elements for Mercury */
	a0=178.179078;
	a1=149474.07078;
	a2=0.0003011;
	a3=0.0;
	l = poly(a0,a1,a2,a3,T0);
	l = range(l);
	a = 0.3870986;
	a0 = 0.20561421;
	a1 = 0.00002046;
	a2 = -0.000000030;
	e = poly(a0,a1,a2,a3,T0);
	a0 = 7.002881;
	a1 = 0.0018608;
	a2 = -0.0000183;
	i = poly(a0,a1,a2,a3,T0);
	a0 = 28.753753;
	a1 = 0.3702806;
	a2 = 0.0001208;
	w1 = poly(a0,a1,a2,a3,T0);
	a0 = 47.145944;
	a1 = 1.1852083;
	a2= 0.0001739;
	w2 = poly(a0,a1,a2,a3,T0);
	M1 = 102.27938 + 149472.51529*T0 + 0.000007*T0*T0;
	M1 = range(M1);
/* test print our of Mercury Coordinates 
	printf("\n L=%f \n a=%f",l,a);
	printf("\n e=%f \n i=%f",e,i);
	printf("\n omega1=%f \n omega2=%f\n",w1,w2);
	printf("mean anomaly=%f",M1);
   end test print for Mercury */
	
/* solving Kepler find the eccentric anomaly ECC    */

	ECC =kepler(e,M1);
	nu = truean(e,ECC);
	r = a*(1.0 - e * cos(ECC*pie/180.0));
	u = l + nu - M1 - w2;
	ll = longi(w2,i,u);
	b = lati(u,i);
/* Now start more test print of intermediate values  
	printf("\n Eccentric anomaly =%f\n",ECC);
	printf("True anaom=%f\n",nu);
	printf("Radius vector=%f\n",r);
	printf("Argu to Latit=%f\n",u);
	printf("eclipitic longitiude=%f\n,",ll);
	printf("eclipitic latitude=%f\n",b);
   end test print */
	
/* Now make corrections due to perturbations */
	M2 = 212.60322 + 58517.80387*T0 +0.001286*T0*T0;
	M2 = range(M2);
	M4 = 319.51913 + 19139.85475*T0 + 0.000181*T0*T0;
	M4 = range(M4);
	M5 = 255.32833 + 3034.69202*T0 - 0.000722*T0*T0;
	M5 = range(M5);
	M6 = 175.46622 +1211.55147*T0 - 0.000502*T0*T0;
	M6 = range(M6);
	lonpert = 0.00204*cos((5*M2-2*M1+12.220)*rad)
		 +0.00103*cos((2*M2-M1-160.692)*rad)
		 +0.00091*cos((2*M5-M1-37.003)*rad)
		 +0.00078*cos((5*M2-3*M1+10.037)*rad);
		
	radpert = 0.000007525*cos((2*M5-M1+53.013)*rad)
		 +0.000006802*cos((5*M2-3*M1-259.918)*rad)
		 +0.000005457*cos((2*M2-2*M1-71.188)*rad)
		 +0.000003569*cos((5*M2-M1-77.75)*rad);
		
		r = r + radpert;
		ll = ll + lonpert;
		
/* Now find the Longi and radius vector of the sun */
		
		M= 358.47583 + 35999.04975*T0 - 0.000150*T0*T0
		   -0.0000033*T0*T0*T0;
		M = range(M);
		esun = 0.01675104 - 0.0000418*T0 -0.000000126*T0*T0;
		Lsun = 279.69668 + 36000.76892*T0 + 0.0003025*T0*T0;
		Lsun = range(Lsun);
		Cen= (1.919460-0.004789*T0-0.000014*T0*T0)*sin(M*rad)
		     +(0.020094 - 0.000100*T0)*sin(2*M*rad)
		     +0.000293*sin(3*M*rad);
		
		Stheta= Lsun + Cen;
		Stheta = range(Stheta);
		Snu = M + Cen;
		Sr = 1.0000002*(1.0 - esun*esun)/(1.0 + esun*cos(Snu*rad));

		omeg= 259.18 - 1934.142*T0;
		thapp = Stheta - 0.00569 - 0.00479* sin(omeg*rad);
		epli = 23.452294 - 0.0130125*T0
		       -0.000001*T0*T0 + 0.000000503*T0*T0*T0
			+0.00256*cos(omeg*rad);
		printf("\nOBJECT     R.A.           DEC    \t DISTANCE");
		printf("\nSun   ");
		N = cos(epli*rad)*sin(thapp*rad);
		D = cos(thapp*rad);
		RA = atan2(N,D)/rad;
		DEC = asin(sin(epli*rad)*sin(thapp*rad))/rad;
		speak(RA,DEC,Sr);
/* tansformation of coordinates on Mercury and ouput */
		printf("\nMer   ");
		trans(r,b,ll,Stheta,Sr,epli);
		
/* Now start on Venus */
	a0 = 342.767053;
	a1 = 58519.21191;
	a2 = 0.0003097;
	a3 = 0.0;
	l = poly(a0,a1,a2,a3,T0);
	l = range(l);
	a = 0.7233316;
	a0 = 0.00682069;
	a1 = -0.00004774;
	a2 = 0.000000091;
	e = poly(a0,a1,a2,a3,T0);
	a0 = 3.393631;
	a1 = 0.0010058;
	a2 = -0.0000010;
	i = poly(a0,a1,a2,a3,T0);
	a0 = 54.384186;
	a1 = 0.5081861;
	a2 = -0.0013864;
	w1 = poly(a0,a1,a2,a3,T0);
	a0 = 75.779647;
	a1 = 0.8998500;
	a2 = 0.0004100;
	w2 = poly(a0,a1,a2,a3,T0);
/* Venus has a long period pert that needs to be added before Kelper */
	lonpert = 0.00077 *sin((237.24 + 150.27*T0)*rad);
	l = l + lonpert;
	M0 = M2 + lonpert;
	ECC = kepler(e,M0);
	nu = truean(e,ECC);
	r = a*(1.0 - e * cos(ECC*rad));
	u = l + nu - M0- w2;
	ll = longi(w2,i,u);
	b = lati(u,i);
	
/* now Venus perturbations */
	
	lonpert = 0.00313*cos((2*M-2*M2 -148.225)*rad)
		 +0.00198*cos((3*M-3*M2 +2.565)*rad)
		 +0.00136*cos((M-M2-119.107)*rad)
		 +0.00096*cos((3*M-2*M2-135.912)*rad)
		 +0.00082*cos((M5-M2-208.087)*rad);
	ll = ll + lonpert;
	
	radpert = 0.000022501 * cos((2*M-2*M2-58.208)*rad)
		 +0.000019045 * cos((3*M-3*M2+92.577)*rad)
		 +0.000006887 * cos((M5-M2-118.090)*rad)
		 +0.000005172 * cos((M-M2-29.110)*rad)
		 +0.000003620 * cos((5*M-4*M2-104.208)*rad)
		 +0.000003283 * cos((4*M-4*M2+63.513)*rad)
		 +0.000003074 * cos((2*M5-2*M2-55.167)*rad);
	r = r + radpert;
	printf("\nVen   ");
	trans(r,b,ll,Stheta,Sr,epli);
	
/* Now  start the planet Mars */
	a0 = 293.737334;
	a1 = 19141.69551;
	a2 = 0.0003107;
	a3 = 0.0;
	l = poly(a0,a1,a2,a3,T0);
	l = range(l);
	a = 1.5236883;
	a0 = 0.09331290;
	a1 = 0.000092064;
	a2 = -0.000000077;
	e = poly(a0,a1,a2,a3,T0);
	a0 = 1.85033;
	a1 = -0.0006750;
	a2 = 0.0000126;
	i = poly(a0,a1,a2,a3,T0);
	a0 = 285.431761;
	a1 = 1.0697667;
	a2 = 0.0001313;
	a3 = 0.00000414;
	w1 = poly(a0,a1,a2,a3,T0);
	w1 = range(w1);
	a0 = 48.786442;
	a1 = 0.7709917;
	a2 = -0.0000014;
	a3 = -0.00000533;
	w2 = poly(a0,a1,a2,a3,T0);
	w2 = range(w2);

/* Mars has a long period perturbation */
	lonpert = -0.01133*sin((3*M5-8*M4 +4*M)*rad)
		  -0.00933*cos((3*M5-8*M4 +4*M)*rad);
	l = l + lonpert;
	M0 = M4 + lonpert;
	ECC = kepler(e,M0);
	nu = truean(e,ECC);
	r = a*(1.0 - e * cos(ECC*rad));
	u = l + nu - M0- w2;
	ll = longi(w2,i,u);
	b = lati(u,i);
	
/* Now Mars Perturbations */	
	lonpert = 0.00705*cos((M5-M4-48.985)*rad)
		 +0.00607*cos((2*M5-M4-188.350)*rad)
		 +0.00445*cos((2*M5-2*M4-191.897)*rad)
		 +0.00388*cos((M-2*M4+20.495)*rad)
		 +0.00238*cos((M-M4+35.097)*rad)
		 +0.00204*cos((2*M-3*M4+158.638)*rad)
		 +0.00177*cos((3*M4-M2-57.602)*rad)
		 +0.00136*cos((2*M-4*M4+154.093)*rad)
		 +0.00104*cos((M5+17.618)*rad);
	ll = ll + lonpert;
	
	radpert = 0.000053227*cos((M5-M4+41.1306)*rad)
		 +0.000050989*cos((2*M5-2*M4-101.9847)*rad)  
		 +0.000038278*cos((2*M5-M4-98.3292)*rad)
		 +0.000015996*cos((M-M4-55.555)*rad)
		 +0.000014764*cos((2*M-3*M4+68.622)*rad)
		 +0.000008966*cos((M5-2*M4+43.615)*rad)
		 +0.000007914*cos((3*M5-2*M4-139.737)*rad)
		 +0.000007004*cos((2*M5-3*M4-102.888)*rad)
		 +0.000006620*cos((M-2*M4+113.202)*rad)
		 +0.000004930*cos((3*M5-3*M4-76.243)*rad)
		 +0.000004693*cos((3*M-5*M4+190.603)*rad)
		 +0.000004571*cos((2*M-4*M4+244.702)*rad)
		 +0.000004409*cos((3*M5-M4-115.828)*rad);
	r = r + radpert;
	printf("\nMars  ");
	trans(r,b,ll,Stheta,Sr,epli);
	
/* Now start Jupiter */
	
putchar('\n');
exit(0);
} /* end of program main */


int cls()
{
	int i;
	for (i=0;i<30;i++)
	{
		putchar('\n');
	}
}

double poly(a0,a1,a2,a3,t)
	double a0,a1,a2,a3,t;
{
	return(a0+a1*t+a2*t*t+a3*t*t*t);
}
double aint(z)
	double z;
{
	int trunk;
	
	trunk = (int)z;
	z = (double) trunk;
	return(z);
}
double kepler(e,M)
	double e,M;
{
	double corr,e0,E0,E1;
	e0 = e /rad;
	corr = 1;
	E0=M;
	while (corr > 0.000001)
	{
		corr = (M + e0* sin(E0*rad) -E0)/(1-e*cos(E0*rad));
		E1 = E0 + corr;
		if (corr < 0)
		{
			corr = -1.0* corr ;
		}
		E0 = E1;
	}
	
	return(E1);
}

double truean(e,E)
	double e,E;
{
	double nu;

	nu = 2.0 * atan(sqrt((1+e)/(1-e))*tan(E*rad/2));
	nu = nu/rad;
	if (nu < 0.0)
	{
		nu = nu + 360.0;
	}
	if (fabs(nu-E) > 90.0)
	{
		if (nu > 180.0)
		{
			nu = nu - 180.0;
		}
		else
		{
			nu = nu + 180.0;
		}
	}
return(nu);
}

double longi(w2,i,u)
	double w2,i,u;
{
	double x,y,l;
	
	y=cos(i*rad)*sin(u*rad);
	x=cos(u*rad);
	l = atan2(y,x);
	l = l/rad;
	if (l < 0.0)
	{
		l = l + 360.0;
	}
	return(l+ w2);
}

double lati(u,i)
	double u,i;
{
	double b;
	b = asin(sin(u*rad)*sin(i*rad))/rad;
	return(b);
}

int speak(ra,dec,dis)
	double ra,dec,dis;
{
	int h,m,s,deg;
	
	if ( ra < 0)
	{
		ra = ra + 360.0;
	}
	ra = ra /15.0; /* convert from degs to hours */
	h = (int) aint(ra);
	m = (int)((ra-h)*60);
	s = (int) (((ra-h)*60-m)*60);
	printf(" %2dh %2dm %2ds ",h,m,s);
	deg = (int)aint(dec);
	m   = (int)((dec - deg)*60);
	s   = (int)(((dec - deg)*60 - m)*60);
	printf("   %+2ddeg %+2dm %+2ds ",deg,m,s);
	printf("\t  %.3f",dis);
	return(0);
}

double range(val)
	double val;
{
	while (val < 0.0)
	{
		val = val + 360.0;
	}
	if (val > 360.0)
	{
		val = val - (aint(val/360.0)*360.0);
	}
	return(val);
}
int trans(r,b,ll,Stheta,Sr,epli)
	double r,b,ll,Stheta,Sr,epli;
{
	double N,D,lambda,delta,beta,RA,DEC;
	
		N = r * cos(b*rad) * sin((ll - Stheta)*rad);
		D = r * cos(b*rad) * cos((ll - Stheta)*rad) + Sr;
		lambda = atan2(N,D)/rad;
		if (lambda < 0.0)
		{
			lambda = lambda + 360.0;
		}
		lambda = range(lambda+Stheta);
		delta = sqrt(N*N + D*D + (r*sin(b*rad))*(r*sin(b*rad)));
		beta = asin(r*sin(b*rad)/delta)/rad;
		N = sin(lambda*rad)*cos(epli*rad)
		    - tan(beta*rad)*sin(epli*rad);
		D = cos(lambda*rad);
		RA = atan2(N,D)/rad;
		DEC = asin(sin(beta*rad)*cos(epli*rad)
		     +cos(beta*rad)*sin(epli*rad)*sin(lambda*rad))/rad;
		speak(RA,DEC,delta);
		return(0);
}
