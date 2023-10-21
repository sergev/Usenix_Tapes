/*  Planets
 *           A program to determine the position of
 *	     the Sun., Mer., Ven., Mars, Jup, Sat & Ura

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
 !
 ! Modified by Alan Paeth, awpaeth@watcgl, January 1987 for
 !   (1) non-interactive mode (uses current GMT)
 !   (2) output in simplified Yale format for use with star charter software
 !
 */

#include <stdio.h>
#include <math.h>

#include <sys/time.h>	/* for getting current GMT */
#define LOGFILE "/u/awpaeth/hobby/astro/planet.star"

/* crude magnitudes of planets (x100) for output filtering by brightness */

#define MAGSOL 0
#define MAGMER 150
#define MAGVEN 0
#define MAGMAR 100
#define MAGJUP 0
#define MAGSAT 100
#define MAGURA 590
#define MAGNEP 800

double pie, rad, kepler(), truean(), longi(), lati(), poly(), aint(), range();
int cls(), trans(), speak();
FILE *logfile;

double usertime()
	{
	int mm,dd,yy,time,tz,aa1,bb1;
	double jd,year,month,day;
	char *zonestr;

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
	printf("\n\nWhat is the time zone correction, local to UT?");
	printf("\nInput 0 for Universal time -\n");
	printf("Input 5 for EST - \n");
	printf("Input 6 for CST - \n");
	printf("Input 7 for MST - \n");
	printf("Input 8 for PST - \n");
	printf("What is the time zone? ");
	scanf("%d",&tz);
	cls();
	printf("For the date %d/%d/%d\n",mm,dd,yy);
	printf(" At %d ",time);
	switch(tz)
	    {
	    case 0:  zonestr = "Universal Time"; break;
	    case 5:  zonestr = "EST"; break;
	    case 6:  zonestr = "CST"; break;
	    case 7:  zonestr = "PST"; break;
	    default: zonestr = "LOCAL TIME"; break;
	    }
	printf("%s\n", zonestr);

/*  Calculate the Julian date for the time of observation */
	
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
	if((year + month/100) > 1582.10) jd = jd + bb1;
        return(jd);
	}

double currenttime()
    {
#define GMT1970 2440587.5
#define SECSPERDAY (60.0*60.0*24.0)
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return(GMT1970 + tv.tv_sec/SECSPERDAY);
    }

main(argc)
    int argc;
    {
	double jd, T0;
	double l,a,e,i,w1,w2,M,M0,M1,M2,M4,M5,M6,M7,M8;
	double a0,a1,a2,a3,RA,DEC;
	double ECC,nu,r,u,ll,b,lonpert,radpert;
	double esun,Lsun,Cen,Stheta,Snu,Sr;
	double N,D,epli,thapp,omeg;
	double nu2,P,Q,S,V,W,ze,l1pert,epert,w1pert,apert;
	double psi,H,G,eta,th;

#define WRITEMODE "w"
#define OPENFAIL 0
	if ((logfile = fopen(LOGFILE, WRITEMODE)) == OPENFAIL)
	    {
	    fprintf(stderr, "fail on opening log file %s\n", LOGFILE);
	    exit(1);
	    }
	cls();
	pie = 3.1415926535898;
	rad = pie/180.0;

/*  calculate time T0 from 1900 January 0.5 ET */
	jd = (argc == 1) ? usertime() : currenttime();
	printf("Julian date: %f\n", jd);
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
	w1 = range(w1);
	a0 = 47.145944;
	a1 = 1.1852083;
	a2= 0.0001739;
	w2 = poly(a0,a1,a2,a3,T0);
	w2 = range(w2);
	M1 = 102.27938 + 149472.51529*T0 + 0.000007*T0*T0;
	M1 = range(M1);
	
/* solving Kepler find the eccentric anomaly ECC    */

	ECC =kepler(e,M1);
	nu = truean(e,ECC);
	r = a*(1.0 - e * cos(ECC*pie/180.0));
	u = l + nu - M1 - w2;
	ll = longi(w2,i,u);
	b = lati(u,i);
	
/* Now make corrections due to perturbations */
	M2 = 212.60322 + 58517.80387*T0 +0.001286*T0*T0;
	M2 = range(M2);
	M4 = 319.51913 + 19139.85475*T0 + 0.000181*T0*T0;
	M4 = range(M4);
	M5 = 225.32833 + 3034.69202*T0 - 0.000722*T0*T0;
	M5 = range(M5);
	M6 = 175.46622 +1221.55147*T0 - 0.000502*T0*T0;
	M6 = range(M6);
	lonpert = 0.00204*cos((5*M2-2*M1+12.220)*rad)
		 +0.00103*cos((2*M2-M1-160.692)*rad)
		 +0.00091*cos((2*M5-M1-37.003)*rad)
		 +0.00078*cos((5*M2-3*M1+10.137)*rad);
		
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
		       -0.00000164*T0*T0 + 0.000000503*T0*T0*T0
			+0.00256*cos(omeg*rad);
		printf("\nOBJECT     R.A.           DEC    \t DISTANCE\n");
		N = cos(epli*rad)*sin(thapp*rad);
		D = cos(thapp*rad);
		RA = atan2(N,D)/rad;
		DEC = asin(sin(epli*rad)*sin(thapp*rad))/rad;
		speak(RA,DEC,Sr, MAGSOL, "PS", "Sol");
/* tansformation of coordinates on Mercury and output */
		trans(r,b,ll,Stheta,Sr,epli, MAGMER, "PM", "Mercury");
		
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
	w1 = range(w1);
	a0 = 75.779647;
	a1 = 0.8998500;
	a2 = 0.0004100;
	w2 = poly(a0,a1,a2,a3,T0);
	w2 = range(w2);
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
	trans(r,b,ll,Stheta,Sr,epli, MAGVEN, "PV", "Venus");
	
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
	a0 = 1.850333;
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
	lonpert = 0.00705*cos((M5-M4-48.958)*rad)
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
	trans(r,b,ll,Stheta,Sr,epli, MAGMAR, "PM", "Mars");
	
/* Now start Jupiter */
	a0 = 238.049257;
	a1 = 3036.301986;
	a2 = 0.0003347;
	a3 = -0.00000165;
	l = poly(a0,a1,a2,a3,T0);
	l = range(l);
	a = 5.202561;
	a0 = 0.04833475;
	a1 = 0.000164180;
	a2 = -0.0000004676;
	a3 = -0.0000000017;
	e = poly(a0,a1,a2,a3,T0);
	a0 = 1.308736;
	a1 = -0.0056961;
	a2 = 0.0000039;
	a3 = 0.0;
	i = poly(a0,a1,a2,a3,T0);
	a0 = 273.277558;
	a1 = 0.5994317;
	a2 = 0.00070405;
	a3 = 0.00000508;
	w1 = poly(a0,a1,a2,a3,T0);
	w1 = range(w1);
	a0 = 99.443414;
	a1 = 1.0105300;
	a2 = 0.00035222;
	a3 = -0.00000851;
	w2 = poly(a0,a1,a2,a3,T0);
	w2 = range(w2);
/*
	Now start perturbation calclations */
	
	nu2 = T0/5.0 +0.1;
	P = 237.47555 +3034.9061*T0;
	Q = 265.91650 + 1222.1139*T0;
	S = 243.51721 + 428.4677*T0;
	V = 5.0*Q -2.0*P;
	W = 2.0*P - 6.0*Q +3.0*S;
	ze = Q - P;
	psi = S - Q;
	
	P = range(P)*rad;
	Q = range(Q)*rad;
	S = range(S)*rad;
	V = range(V)*rad;
	W = range(W)*rad;
	ze = range(ze)*rad;
	psi = range(psi)*rad;
	
l1pert = (0.331364 - 0.010281*nu2 - 0.004692*nu2*nu2)*sin(V)
	+(0.003228 - 0.064436*nu2 + 0.002075*nu2*nu2)*cos(V)
	-(0.003083 + 0.000275*nu2 - 0.000489*nu2*nu2)*sin(2*V)
	+0.002472*sin(W)
	+0.013619*sin(ze)
	+0.018472*sin(2*ze)
	+0.006717*sin(3*ze)
	+0.002775*sin(4*ze)
	+(0.007275 - 0.001253*nu2)*sin(ze)*sin(Q)
	+0.006417*sin(2*ze)*sin(Q)
	+0.002439*sin(3*ze)*sin(Q);
	
l1pert = l1pert	-(0.033839 + 0.001125*nu2)*cos(ze)*sin(Q)
	-0.003767*cos(2*ze)*sin(Q)
	-(0.035681 + 0.001208*nu2)*sin(ze)*cos(Q)
	-0.004261*sin(2*ze)*cos(Q)
	+0.002178*cos(Q)
	+(-0.006333 + 0.001161*nu2)*cos(ze)*cos(Q)
	-0.006675*cos(2*ze)*cos(Q)
	-0.002664*cos(3*ze)*cos(Q)
	-0.002572*sin(ze)*sin(2*Q)
	-0.003567*sin(2*ze)*sin(2*Q)
	+0.002094*cos(ze)*cos(2*Q)
	+0.003342*cos(2*ze)*cos(2*Q);
	
epert =  (.0003606 + .0000130*nu2 - .0000043*nu2*nu2)*sin(V)
	+(.0001289 - .0000580*nu2)*cos(V)
	-.0006764*sin(ze)*sin(Q)
	-.0001110*sin(2*ze)*sin(Q)
	-.0000224*sin(3*ze)*sin(Q)
	-.0000204*sin(Q)
	+(.0001284 + .0000116*nu2)*cos(ze)*sin(Q)
	+.0000188*cos(2*ze)*sin(Q)
	+(.0001460 + .0000130*nu2)*sin(ze)*cos(Q)
	+.0000224*sin(2*ze)*cos(Q)
	-.0000817*cos(Q);
	
epert = epert	+.0006074*cos(ze)*cos(Q)
	+.0000992*cos(2*ze)*cos(Q)
	+.0000508*cos(3*ze)*cos(Q)
	+.0000230*cos(4*ze)*cos(Q)
	+.0000108*cos(5*ze)*cos(Q)
	-(.0000956 + .0000073*nu2)*sin(ze)*sin(2*Q)
	+.0000448*sin(2*ze)*sin(2*Q)
	+.0000137*sin(3*ze)*sin(2*Q)
	+(-.0000997 + .0000108*nu2)*cos(ze)*sin(2*Q)
	+.0000480*cos(2*ze)*sin(2*Q);

epert = epert	+.0000148*cos(3*ze)*sin(2*Q)
	+(-.0000956 +.0000099*nu2)*sin(ze)*cos(2*Q)
	+.0000490*sin(2*ze)*cos(2*Q)
	+.0000158*sin(3*ze)*cos(2*Q)
	+.0000179*cos(2*Q)
	+(.0001024 + .0000075*nu2)*cos(ze)*cos(2*Q)
	-.0000437*cos(2*ze)*cos(2*Q)
	-.0000132*cos(3*ze)*cos(2*Q);
	
w1pert = (0.007192 - 0.003147*nu2)*sin(V)
	+(-0.020428 - 0.000675*nu2 + 0.000197*nu2*nu2)*cos(V)
	+(0.007269 + 0.000672*nu2)*sin(ze)*sin(Q)
	-0.004344*sin(Q)
	+0.034036*cos(ze)*sin(Q)
	+0.005614*cos(2*ze)*sin(Q)
	+0.002964*cos(3*ze)*sin(Q)
	+0.037761*sin(ze)*cos(Q);
	
w1pert = w1pert
	+0.006158*sin(2*ze)*cos(Q)
	-0.006603*cos(ze)*cos(Q)
	-0.005356*sin(ze)*sin(2*Q)
	+0.002722*sin(2*ze)*sin(2*Q)
	+0.004483*cos(ze)*sin(2*Q)
	-0.002642*cos(2*ze)*sin(2*Q)
	+0.004403*sin(ze)*cos(2*Q)
	-0.002536*sin(2*ze)*cos(2*Q)
	+0.005547*cos(ze)*cos(2*Q)
	-0.002689*cos(2*ze)*cos(2*Q);
	
lonpert = l1pert -(w1pert/e);
l = l + l1pert;
M0 = M5 + lonpert;
e = e + epert;
w1 = w1 + w1pert;

apert =  -.000263*cos(V)
	+.000205*cos(ze)
	+.000693*cos(2*ze)
	+.000312*cos(3*ze)
	+.000147*cos(4*ze)
	+.000299*sin(ze)*sin(Q)
	+.000181*cos(2*ze)*sin(Q)
	+.000204*sin(2*ze)*cos(Q)
	+.000111*sin(3*ze)*cos(Q)
	-.000337*cos(ze)*cos(Q)
	-.000111*cos(2*ze)*cos(Q);
	
a = a + apert;
ECC = kepler(e,M0);
nu = truean(e,ECC);
r = a*(1.0 - e * cos(ECC*rad));
u = l + nu - M0 - w2;
ll = longi(w2,i,u);
b = lati(u,i);

trans(r,b,ll,Stheta,Sr,epli, MAGJUP, "PJ", "Jupiter");

/* Now start Saturn */

	a0 = 266.564377;
	a1 = 1223.509884;
	a2 = 0.0003245;
	a3 = -0.0000058;
	l = poly(a0,a1,a2,a3,T0);
	l = range(l);
	a = 9.554747;
	a0 = 0.05589232;
	a1 = -0.00034550;
	a2 = -0.000000728;
	a3 = 0.00000000074;
	e = poly(a0,a1,a2,a3,T0);
	a0 = 2.492519;
	a1 = -0.0039189;
	a2 = -0.00001549;
	a3 = 0.00000004;
	i = poly(a0,a1,a2,a3,T0);
	a0 = 338.307800;
	a1 = 1.0852207;
	a2 = 0.00097854;
	a3 = 0.00000992;
	w1 = poly(a0,a1,a2,a3,T0);
	w1 = range(w1);
	a0 = 112.790414;
	a1 = 0.8731951;
	a2 = -0.00015218;
	a3 = -0.00000531;
	w2 = poly(a0,a1,a2,a3,T0);
	w2 = range(w2);
	
/* Now Saturn's perturbations */
	
l1pert = (-0.814181 + 0.018150*nu2 + 0.016714*nu2*nu2)*sin(V)
	 +(-0.010497 + 0.160906*nu2 - 0.004100*nu2*nu2)*cos(V)
	 +0.007581*sin(2*V)
	 -0.007986*sin(W)
	 -0.148811*sin(ze)
	 -0.040786*sin(2*ze)
	 -0.015208*sin(3*ze)
	 -0.006339*sin(4*ze)
	 -0.006244*sin(Q);
l1pert = l1pert 
	+(0.008931 + 0.002728*nu2)*sin(ze)*sin(Q)
	-0.016500*sin(2*ze)*sin(Q)
	-0.005775*sin(3*ze)*sin(Q)
	+(0.081344 + 0.003206*nu2)*cos(ze)*sin(Q)
	+0.015019*cos(2*ze)*sin(Q)
	+(0.085581 + 0.002494*nu2)*sin(ze)*cos(Q)
	+(0.025328 - 0.003117*nu2)*cos(ze)*cos(Q);
l1pert = l1pert 
	+0.014394*cos(2*ze)*cos(Q)
	+0.006319*cos(3*ze)*cos(Q)
	+0.006369*sin(ze)*sin(2*Q)
	+0.009156*sin(2*ze)*sin(2*Q)
	+0.007525*sin(3*psi)*sin(2*Q)
	-0.005236*cos(ze)*cos(2*Q)
	-0.007736*cos(2*ze)*cos(2*Q)
	-0.007528*cos(3*psi)*cos(2*Q);
	
epert = (-.0007927 + .0002548*nu2 +.0000091*nu2*nu2)*sin(V)
	+(.0013381 + .0001226*nu2 -.0000253*nu2*nu2)*cos(V)
	+(.0000248 - .0000121*nu2)*sin(2*V)
	-(.0000305 + .0000091*nu2)*cos(2*V)
	+.0000412*sin(2*ze)
	+.0012415*sin(Q)
	+(.0000390 -.0000617*nu2)*sin(ze)*sin(Q)
	+(.0000165 - .0000204*nu2)*sin(2*ze)*sin(Q)
	+.0026599*cos(ze)*sin(Q)
	-.0004687*cos(2*ze)*sin(Q);
epert = epert
	-.0001870*cos(3*ze)*sin(Q)
	-.0000821*cos(4*ze)*sin(Q)
	-.0000377*cos(5*ze)*sin(Q)
	+.0000497*cos(2*psi)*sin(Q)
	+(.0000163 - .0000611*nu2)*cos(Q)
	-.0012696*sin(ze)*cos(Q)
	-.0004200*sin(2*ze)*cos(Q)
	-.0001503*sin(3*ze)*cos(Q)
	-.0000619*sin(4*ze)*cos(Q)
	-.0000268*sin(5*ze)*cos(Q);
epert = epert
	-(.0000282 + .0001306*nu2)*cos(ze)*cos(Q)
	+(-.0000086 + .0000230*nu2)*cos(2*ze)*cos(Q)
	+.0000461*sin(2*psi)*cos(Q)
	-.0000350*sin(2*Q)
	+(.0002211 - .0000286*nu2)*sin(ze)*sin(2*Q)
	-.0002208*sin(2*ze)*sin(2*Q)
	-.0000568*sin(3*ze)*sin(2*Q)
	-.0000346*sin(4*ze)*sin(2*Q)
	-(.0002780 + .0000222*nu2)*cos(ze)*sin(2*Q)
	+(.0002022 + .0000263*nu2)*cos(2*ze)*sin(2*Q);
epert = epert
	+.0000248*cos(3*ze)*sin(2*Q)
	+.0000242*sin(3*psi)*sin(2*Q)
	+.0000467*cos(3*psi)*sin(2*Q)
	-.0000490*cos(2*Q)
	-(.0002842 + .0000279*nu2)*sin(ze)*cos(2*Q)
	+(.0000128 + .0000226*nu2)*sin(2*ze)*cos(2*Q)
	+.0000224*sin(3*ze)*cos(2*Q)
	+(-.0001594 + .0000282*nu2)*cos(ze)*cos(2*Q)
	+(.0002162 - .0000207*nu2)*cos(2*ze)*cos(2*Q)
	+.0000561*cos(3*ze)*cos(2*Q);
epert = epert
	+.0000343*cos(4*ze)*cos(2*Q)
	+.0000469*sin(3*psi)*cos(2*Q)
	-.0000242*cos(3*psi)*cos(2*Q)
	-.0000205*sin(ze)*sin(3*Q)
	+.0000262*sin(3*ze)*sin(3*Q)
	+.0000208*cos(ze)*cos(3*Q)
	-.0000271*cos(3*ze)*cos(3*Q)
	-.0000382*cos(3*ze)*sin(4*Q)
	-.0000376*sin(3*ze)*cos(4*Q);

w1pert = (0.077108 + 0.007186*nu2 - 0.001533*nu2*nu2)*sin(V)
	+(0.045803 - 0.014766*nu2 - 0.000536*nu2*nu2)*cos(V)
	-0.007075*sin(ze)
	-0.075825*sin(ze)*sin(Q)
	-0.024839*sin(2*ze)*sin(Q)
	-0.008631*sin(3*ze)*sin(Q)
	-0.072586*cos(Q)
	-0.150383*cos(ze)*cos(Q)
	+0.026897*cos(2*ze)*cos(Q)
	+0.010053*cos(3*ze)*cos(Q);
w1pert = w1pert
	-(0.013597 +0.001719*nu2)*sin(ze)*sin(2*Q)
	+(-0.007742 + 0.001517*nu2)*cos(ze)*sin(2*Q)
	+(0.013586 - 0.001375*nu2)*cos(2*ze)*sin(2*Q)
	+(-0.013667 + 0.001239*nu2)*sin(ze)*cos(2*Q)
	+0.011981*sin(2*ze)*cos(2*Q)
	+(0.014861 + 0.001136*nu2)*cos(ze)*cos(2*Q)
	-(0.013064 + 0.001628*nu2)*cos(2*ze)*cos(2*Q);
	
lonpert = l1pert -(w1pert/e);
l = l + l1pert;
M0 = M6 + lonpert;
e = e + epert;
w1 = w1 + w1pert;

apert = .000572*sin(V) -.001590*sin(2*ze)*cos(Q)
	+.002933*cos(V) -.000647*sin(3*ze)*cos(Q)
	+.033629*cos(ze) -.000344*sin(4*ze)*cos(Q)
	-.003081*cos(2*ze) +.002885*cos(ze)*cos(Q)
	-.001423*cos(3*ze) +(.002172 + .000102*nu2)*cos(2*ze)*cos(Q)
	-.000671*cos(4*ze) +.000296*cos(3*ze)*cos(Q)
	-.000320*cos(5*ze) -.000267*sin(2*ze)*sin(2*Q);
apert = apert
	+.001098*sin(Q) -.000778*cos(ze)*sin(2*Q)
	-.002812*sin(ze)*sin(Q) +.000495*cos(2*ze)*sin(2*Q)
	+.000688*sin(2*ze)*sin(Q) +.000250*cos(3*ze)*sin(2*Q);
apert = apert
	-.000393*sin(3*ze)*sin(Q)
	-.000228*sin(4*ze)*sin(Q)
	+.002138*cos(ze)*sin(Q)
	-.000999*cos(2*ze)*sin(Q)
	-.000642*cos(3*ze)*sin(Q)
	-.000325*cos(4*ze)*sin(Q)
	-.000890*cos(Q)
	+.002206*sin(ze)*cos(Q);
apert = apert
	-.000856*sin(ze)*cos(2*Q)
	+.000441*sin(2*ze)*cos(2*Q)
	+.000296*cos(2*ze)*cos(2*Q)
	+.000211*cos(3*ze)*cos(2*Q)
	-.000427*sin(ze)*sin(3*Q)
	+.000398*sin(3*ze)*sin(3*Q)
	+.000344*cos(ze)*cos(3*Q)
	-.000427*cos(3*ze)*cos(3*Q);
	
a = a + apert;
ECC = kepler(e,M0);
nu = truean(e,ECC);
r = a*(1.0 - e * cos(ECC*rad));
u = l + nu - M0 - w2;
ll = longi(w2,i,u);
b = lati(u,i);

b = b
	+0.000747*cos(ze)*sin(Q)
	+0.001069*cos(ze)*cos(Q)
	+0.002108*sin(2*ze)*sin(2*Q)
	+0.001261*cos(2*ze)*sin(2*Q)
	+0.001236*sin(2*ze)*cos(2*Q)
	-0.002075*cos(2*ze)*cos(2*Q);

trans(r,b,ll,Stheta,Sr,epli, MAGSAT, "PS", "Saturn");

/* Now Start on Uranus */
	a0 = 244.197470;
	a1 = 429.863546;
	a2 = 0.0003160;
	a3 = -0.00000060;
	l = poly(a0,a1,a2,a3,T0);
	l = range(l);
	a = 19.21814;
	a0 = 0.0463444;
	a1 = -0.00002658;
	a2 = 0.000000077;
	a3 = 0.0;
	e = poly(a0,a1,a2,a3,T0);
	a0 = 0.772464;
	a1 = 0.0006253;
	a2 = 0.0000395;
	i = poly(a0,a1,a2,a3,T0);
	a0 = 98.071581;
	a1 = 0.9857650;
	a2 = -0.0010745;
	a3 = -0.00000061;
	w1 = poly(a0,a1,a2,a3,T0);
	w1 = range(w1);
	a0 = 73.477111;
	a1 = 0.4986678;
	a2 = 0.0013117;
	a3 = 0.0;
	w2 = poly(a0,a1,a2,a3,T0);
	w2 = range(w2);
	M7 = 72.64878 + 428.37911*T0 + 0.000079*T0*T0;
	M7 = range(M7);
/* now perturbation corrections for Uranus */
	G = 83.76922 + 218.4901*T0;
	S = S/rad;
	P = P/rad;
	Q = Q/rad;
	H = 2.0*G - S;
	ze = S - P;
	eta = S - Q;
	th = G - S;
	S = S*rad;
	G = range(G)*rad;
	P = P*rad;
	Q = Q*rad;
	H = range(H)*rad;
	ze = range(ze)*rad;
	eta = range(eta)*rad;
	th = range(th)*rad;
	
l1pert = (0.864319 - 0.001583*nu2)*sin(H)
	+(0.082222 - 0.006833*nu2)*cos(H)
	+0.036017*sin(2*H)
	-0.003019*cos(2*H)
	+0.008122*sin(W);
	
epert = (-.0003349 + .0000163*nu2)*sin(H)
	+.0020981*cos(H)
	+.0001311*cos(H);
	
w1pert = 0.120303*sin(H)
	+(0.019472 - 0.000947*nu2)*cos(H)
	+0.006197*sin(2*H);
	
lonpert = l1pert -(w1pert/e);
l = l + l1pert;
M0 = M7 + lonpert;
e = e + epert;
w1 = w1 + w1pert;

a = a - 0.003825*cos(H);
ECC = kepler(e,M0);
nu = truean(e,ECC);
r = a*(1.0 - e * cos(ECC*rad));
u = l + nu - M0 - w2;
ll = longi(w2,i,u);
b = lati(u,i);

ll = ll
	+(0.010122 - 0.000988*nu2)*sin(S+eta)
	+(-0.038581 + 0.002031*nu2 - 0.001910*nu2*nu2)*cos(S+eta)
	+(0.034964 - 0.001038*nu2 + 0.000868*nu2*nu2)*cos(2*S+eta)
	+0.005594*sin(S +3*th);
ll = ll
	-0.014808*sin(ze)
	-0.005794*sin(eta)
	+0.002347*cos(eta)
	+0.009872*sin(th)
	+0.008803*sin(2*th)
	-0.004308*sin(3*th);
	
b = b 
	+(0.000458*sin(eta) - 0.000642*cos(eta) - 0.000517*cos(4*th))
	*sin(S)
	-(0.000347*sin(eta) + 0.000853*cos(eta) + 0.000517*sin(4*eta))
	*cos(S)
	+0.000403*(cos(2*th)*sin(2*S) + sin(2*th)*cos(2*S));
	
r = r
	-.025948
	+.004985*cos(ze)
	-.001230*cos(S)
	+.003354*cos(eta)
	+(.005795*cos(S) - .001165*sin(S) + .001388*cos(2*S))*sin(eta)
	+(.001351*cos(S) + .005702*sin(S) + .001388*sin(2*S))*cos(eta)
	+.000904*cos(2*th)
	+.000894*(cos(th) - cos(3*th));
trans(r,b,ll,Stheta,Sr,epli, MAGURA, "PU", "Uranus");

/* Now start Neptune */
	a0 = 84.457994;
	a1 = 219.885914;
	a2 = 0.0003205;
	a3 = -0.00000060;
	l = poly(a0,a1,a2,a3,T0);
	l = range(l);
	a = 30.10957;
	a0 = 0.00899704;
	a1 = 0.000006330;
	a2 = -0.000000002;
	a3 = 0.0;
	e = poly(a0,a1,a2,a3,T0);
	a0 = 1.779242;
	a1 = -0.0095436;
	a2 = -0.0000091;
	i = poly(a0,a1,a2,a3,T0);
	a0 = 276.045975;
	a1 = 0.3256394;
	a2 = 0.00014095;
	a3 = 0.000004113;
	w1 = poly(a0,a1,a2,a3,T0);
	w1 = range(w1);
	a0 = 130.681389;
	a1 = 1.0989350;
	a2 = 0.00024987;
	a3 = -0.000004718;
	w2 = poly(a0,a1,a2,a3,T0);
	w2 = range(w2);
	M8 = 37.73063 + 218.46134*T0 -0.000070*T0*T0;
	
/* now perturbation corrections for neptune */
	G = G/rad;
	P = P/rad;
	Q = Q/rad;
	S = S/rad;
	ze = G - P;
	eta = G - Q;
	th = G - S;
	G = G*rad;
	P = P*rad;
	Q = Q*rad;
	S = S*rad;
	ze = range(ze)*rad;
	eta = range(eta)*rad;
	th = range(th)*rad;
	
l1pert = (-0.589833 + 0.001089*nu2)*sin(H)
	+(-0.056094 + 0.004658*nu2)*cos(H)
	-0.024286*sin(2*H);
	
epert = .0004389*sin(H)
	+.0004262*cos(H)
	+.0001129*sin(2*H)
	+.0001089*cos(2*H);
	
w1pert = 0.024039*sin(H)
	-0.025303*cos(H)
	+0.006206*sin(2*H)
	-0.005992*cos(2*H);
	
	
lonpert = l1pert -(w1pert/e);
l = l + l1pert;
M0 = M8 + lonpert;
e = e + epert;
w1 = w1 + w1pert;

a = a - 0.000817*sin(H)
	+0.008189*cos(H)
	+0.000781*cos(2*H);
	
ECC = kepler(e,M0);
nu = truean(e,ECC);
r = a*(1.0 - e * cos(ECC*rad));
u = l + nu - M0 - w2;
ll = longi(w2,i,u);
b = lati(u,i);

ll = ll
	-0.009556*sin(ze)
	-0.005178*sin(eta)
	+0.002572*sin(2*th)
	-0.002972*cos(2*th)*sin(G)
	-0.002833*sin(2*th)*cos(G);
	
b = b 
	+0.000336*cos(2*th)*sin(G)
	+0.000364*sin(2*th)*cos(G);
	
r = r
	-.040596
	+.004992*cos(ze)
	+.002744*cos(eta)
	+.002044*cos(th)
	+.001051*cos(2*th);
trans(r,b,ll,Stheta,Sr,epli, MAGNEP, "PN", "Neptune");

putchar('\n');
close(logfile);
exit(0);
} /* end of program main */


int cls()
    {
#define CLEARCNT 1
    int i;
    for (i=0; i < CLEARCNT; i++) putchar('\n');
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

int speak(ra,dec,dis, mag, sym, str)
	double ra,dec,dis;
	char *sym, *str;
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
	printf("%s\t%2dh %2dm %2ds ",str, h,m,s);
	fprintf(logfile, "%02d%02d%02d", h, m, s);
	deg = (int)aint(dec);
	m   =(int)((dec - deg)*60);
	s   = (int) (((dec - deg)*60 - m)*60);
	if ( m < 0)
	{
		m = m * -1;
	}
	if (s < 0)
	{
		s = s * -1;
	}
	printf("   %+3ddeg %2dm %2ds ",deg,m,s);
	printf("\t  %.3f\n",dis);
	fprintf(logfile, "%+03d%02d+%03d%s%s\n", deg, m, mag, sym, str);
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
int trans(r,b,ll,Stheta,Sr,epli, mag, sym, str)
	double r,b,ll,Stheta,Sr,epli;
	char *sym, *str;
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
		speak(RA,DEC,delta, mag, sym, str);
		return(0);
}
