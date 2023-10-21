/* <sdate.c>
 * Compute various useful times
 *
 * Written by Marc T. Kaufman
 *            14100 Donelson Place
 *            Los Altos Hills, CA 94022
 *            (415) 948-3777
 *
 * Based on : "Explanatory Supplement to the Astronomical Ephemeris
 *             and the American Ephemeris and Nautical Almanac",
 *             H.M. Nautical Almanac Office, London.  Updated from
 *             equations in the 1985 Astronomical Almanac.
 *
 * Copyright 1986 by Marc Kaufman
 *
 * Permission to use this program is granted, provided it is not sold.
 *
 * This program was originally written on a VAX, under 4.2bsd.
 *  it was then ported to a 68000 system under REGULUS (Alcyon's version
 *  of UNIX system III).  Major differences included: no 'double' and
 *  a default integer length of 'short'.  Having been through all that,
 *  porting to your machine should be easy.  Watch out for 'time' related
 *  functions and make sure your 'atan2' program works right.
 *
 *	850210	revised to 1985 Ephemeris - mtk
 */

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <math.h>

long	UTC, TDT, tim, tim2, localdst;
double	Julian_Day, MJD, Tu, Ru, T70, Local, GMST, LST;
double	Eqt, Tua, L, G, e, eps, g, alpha, delta, sd, cd, lha, lhr, sh, ch;
double	la, lf, S, C, sp, cp, tp, Az, alt;
double	Lm, lm, px, SD, am, dm;
double	zs, x;
double	fabs(), fmod(), asin(), acos();
struct	tm *t, *Rlocaltime(), *gmtime();
char	*tdate, *gmctime(), *localctime();
int	ftime();
struct	timeb tb;

#define Pi			3.1415926535
#define Degree_to_Radian	((2.0 * Pi)/ 360.)
#define Asec_Radian		((2.0 * Pi)/(360. * 60. * 60.))
#define Tsec_to_Radian		((2.0 * Pi)/( 24. * 60.* 60.))
#define Asec_to_Tsec		(24./360.)
#define Sec_per_day		(24 * 60 * 60)
#define Round			0.5		/* for rounding to integer */

#define J1900	/* 24 */15020.0	/* Julian Day number at Epoch 1900.0 */
#define	J1970	/* 24 */40587.5	/* VAX clock Epoch 1970 Jan 1 (0h UT) */
#define	J1985	/* 24 */46065.5	/* Epoch 1985 Jan 1 (0h UT) */
#define	J2000	/* 24 */51545.0	/* Epoch 2000 Jan 1 (12h UT) */
#define Delta_T		(54.6 + 0.9*(Julian_Day - J1985)/365.)	/* TDT - UT */
/* (This is the position of my house ) */
#define Longitude	(((122.)*60. +  8.)*60. +  3.)	/* Arc-seconds West */
#define Latitude	((( 37.)*60. + 22.)*60. + 58.)	/* Arc-seconds North */
#define f1			(1. - (1./298.25))	/* 1 - flattening of Earth */
/* the following alternate values are useful when debugging */
/*#define Longitude	(((000.)*60. +  0.)*60. +  0.)	/* Arc-seconds West */
/*#define Latitude	((( 35.)*60. +  0.)*60. +  0.)	/* Arc-seconds North */
/*#define f1		1.			/* 1 - flattening of Earth */

main() {

/* at this point we digress to discuss UNIX differences.
 * In UCB UNIX we dont have ctime(), but do instead have asctime(),
 *  which works from the structures created by gmtime() and localtime().
 * However, system time is kept in UTC (Greenwich), and the localtime
 *  routine correctly handles daylight savings time.
 * Since the Regulus system only knows local time, a few direct
 *  fiddles are needed.
 */

/* correct apparent latitude for shape of Earth */

	lf= atan(f1*f1 * tan(Latitude * Asec_Radian));
	sp= sin(lf);
	cp= cos(lf);
	tp= sp/cp;

	time(&UTC);			/* get time */
	Local= - Longitude/15.;		/* Local apparent time correction */

	{ int h, m, s;		/* manual entry mode */
/*	time(&tim);
	t= gmtime(&tim);
	tim= tim - (60 * (60 * t->tm_hour + t->tm_min) + t->tm_se);
	scanf("%d %d %d", &h, &m, &s);
	{UTC = tim + 60 * (60 * h + m) + s;
*/	}

/* !	t= gmtime(&UTC);	/* this is Regulus time */
	t= localtime(&UTC);	/* VAX version */

/* Compute delta to real UTC from time zone time */

	/* do this by hand since Regulus wont */

	switch (t->tm_mon + 1)	/* months are numbered from 0 */
	{
		case 1:
		case 2:
		case 3:
		case 11:
		case 12:
			t->tm_isdst = 0;
			break;

		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			t->tm_isdst = 1;
			break;

		case 4:
			if ((t->tm_mday < 24) || (t->tm_mday - t->tm_wday <= 24))
				t->tm_isdst = 0;
			else
				t->tm_isdst = 1;
			break;

		case 10:
			if ((t->tm_mday < 25) || (t->tm_mday - t->tm_wday <= 25))
				t->tm_isdst = 1;
			else
				t->tm_isdst = 0;
			break;
	}
	ftime(&tb);			/* gets time-zone information */
	if (tb.dstflag == 0)
		t->tm_isdst = 0;	/* dst never used here */
	localdst = (-tb.timezone + t->tm_isdst*60) * 60L;	/* local time correction */

/* !	UTC -= localdst;	/* this is real UTC, not what the OS gave us! */
	printf("%.24s GMT\n", gmctime(&UTC));

	stuff(UTC);			/* start with local time info */

/* Compute Terrestrial Dynamical Time (this used to be called Ephemeris Time) */

	TDT = UTC + (long)(Delta_T + Round);
	tdate= gmctime(&TDT);
	printf("           %.8s      Terrestrial Dynamical Time\n", tdate+11);

	printf("%.24s Local Civil Time\n", localctime(&UTC));

	tim2 = UTC + (long)(Local + Round);	/* Compute Local Solar Time */
	tdate= gmctime(&tim2);
	printf("           %.8s      Local Mean Time\n", tdate+11);

/* compute phase of moon */

	moondata(UTC);
	Lm = fmod(Lm-L, 360.);	/* phase is Lm - L (longitude of Sun) */
	lm = fmod(Lm, 90.);	/* excess over phase boundary */
	printf("The Moon is%4.1f days past ", lm*36525./481267.883);
	if	(Lm <  90.)	printf("New\n");
	else if (Lm < 180.)	printf("First Quarter\n");
	else if (Lm < 270.)	printf("Full\n");
	else			printf("Last Quarter\n");

	printf("Julian Day  24%9.3f\n", Julian_Day);

	tim2 = GMST + Round;
	tdate= gmctime(&tim2);
		printf("           %.8s      Greenwich Mean Sidereal Time\n", tdate+11);

	tim2 = LST + Round;
	tdate= gmctime(&tim2);
		printf("           %.8s      Local Sidereal Time\n", tdate+11);

	tim2= lha + Round;
	tdate= gmctime(&tim2);
		printf("           %.8s      L.H.A. of Sun\n", tdate+11);
		printf("            %11.3f  Degrees Declnation\n",delta/3600.);
		printf("Azimuth     %11.3f  Degrees\n",Az/3600.);
		printf("Elevation   %11.3f  Degrees\n",alt/3600.);

	/* compute sunrise and sunset */
	t= Rlocaltime(&UTC);		/* compute start of day */
	tim = UTC - (3600L * t->tm_hour + 60L * t->tm_min + t->tm_sec)
			+ Sec_per_day/2;	/* about noon */

	zs = 90. + 50./60.;			/* zenith angle of rise/set */
	sunrise(tim, -1.0, zs, "Sunrise ");
		printf("       ");
		sunrise((long)(tim+Sec_per_day), -1.0, zs, "Tomorrow");
		printf("\n");
	sunrise(tim, 1.0, zs, "Sunset  ");
		printf("       ");
		sunrise((long)(tim+Sec_per_day), 1.0, zs, "Tomorrow");
		printf("\n");

	/* compute moonrise and moonset */
	tim = tim - Sec_per_day/2 - 31;	/* about start of day */

	zs = 90. + 34./60.;		/* zenith angle of rise/set */
	moonrise(tim, -1.0, zs, "Moonrise");
		printf("       ");
		moonrise((long)(tim+Sec_per_day), -1.0, zs, "Tomorrow");
		printf("\n");
	moonrise(tim, 1.0, zs, "Moonset ");
		printf("       ");
		moonrise((long)(tim+Sec_per_day), 1.0, zs, "Tomorrow");
		printf("\n");
}

sunrise(t0, rs, z, s)
	long t0;
	double rs, z;
	char *s;
{
	double cz, dh;
	long dt;

	cz = cos(z * Degree_to_Radian);	/* zenith distance of phenomonon */

	do {	/* iterate */
		stuff(t0);	/* compute declination and current hour angle */
		dh= -tp*sd/cd + cz/(cp*cd);
		if ((dh < -1.0) || (dh > 1.0)) {
			printf("%.8s   none   ", s);
			return;
		}
		dh=acos(dh)*rs;
		dt= (dh - lhr) / Tsec_to_Radian;
		t0 += dt;
	} while (dt);

	t0 += 30 /* seconds, rounding to nearest minute */;
	tdate= localctime(&t0);
		printf("%.8s   %.5s  ", s, tdate+11);
}

moonrise(t0, rs, z, s)
	long t0;
	double rs, z;
	char *s;
{
#define SRATE	1.033863192	/* ratio of Moon's motion to Sun's motion */
	double	cz, dh, sd, cd;
	long	t1, dt;

	moondata(t0);	/* get starting declination of Moon */

	/* compute zenith distance of phenomonon */
	cz = cos(z * Degree_to_Radian + SD /* -px */);

	/* first iteraton is forward only (to approx. phenom time) */
	sd = sin(dm);
	cd = cos(dm);
	dh= -tp*sd/cd + cz/(cp*cd);
	if ((dh < -1.0) || (dh > 1.0)) {
		printf("%.8s   none   ", s);
		return;
	}
	dh= acos(dh)*rs;
	dt= fmod((dh - am), 2.0*Pi) * SRATE / Tsec_to_Radian;
	t1 = t0 + dt;

	do {	/* iterate */
		moondata(t1);	/* compute declination and current hour angle */
		cz = cos(z * Degree_to_Radian + SD /* -px */);
		sd = sin(dm);
		cd = cos(dm);

		dh= -tp*sd/cd + cz/(cp*cd);
		if ((dh < -1.0) || (dh > 1.0)) {
			printf("%.8s   none  ", s);
			return;
		}
		dh= acos(dh)*rs;
		dt= (dh - am) * SRATE / Tsec_to_Radian;
		t1 += dt;
	} while (dt);

	if ((t1 - t0) >= Sec_per_day) {
			printf("%.8s   none   ", s);
		return;
	}
	t1 += 30 /* seconds, rounding to nearest minute */;
	tdate= localctime(&t1);
		printf("%.8s   %.5s  ", s, tdate+11);
}

stuff(tim)
long tim;
{		/* main computation loop */

	timedata(tim);

/* where is the Sun (angles are in seconds of arc) */
/*	Low precision elements from 1985 Almanac   */

	L= 280.460 + 0.9856474 * MJD;		/* Mean Longitde */
	L = fmod(L, 360.);		/* corrected for aberration */

	g= 357.528 + 0.9856003 * MJD;		/* Mean Anomaly */
	g = fmod(g, 360.);

	eps= 23.439 - 0.0000004 * MJD;		/* Mean Obiquity of Ecliptic */

	{	/* convert to R.A. and DEC */
		double Lr, gr, epsr, lr, ca, sa, R;
		double sA, cA, gphi;

		Lr = L * Degree_to_Radian;
		gr = g * Degree_to_Radian;
		epsr = eps * Degree_to_Radian;

		lr = (L + 1.915*sin(gr) + 0.020*sin(2.0*gr)) * Degree_to_Radian;

		sd = sin(lr) * sin(epsr);
		cd = sqrt(1.0 - sd*sd);
		sa = sin(lr) * cos(epsr);
		ca = cos(lr);

		delta = asin(sd);
		alpha = atan2(sa, ca);

	/* equation of time */
		Eqt= (Lr - alpha) / Tsec_to_Radian;

		delta = delta / Asec_Radian;
		alpha = alpha / Tsec_to_Radian;

		lhr = (LST - alpha) * Tsec_to_Radian;
		sh =  sin(lhr);
		ch =  cos(lhr);
		lhr= atan2(sh, ch);	/* normalized -pi to pi */
		lha= lhr / Tsec_to_Radian + Sec_per_day/2;

	/* convert to Azimuth and altitude */

		alt = asin(sd*sp + cd*ch*cp);
		ca =  cos(alt);
		sA =  -cd * sh / ca;
		cA =  (sd*cp - cd*ch*sp) / ca;
		Az = atan2(sA, cA) / Asec_Radian;
		Az = fmod(Az, 1296000. /* 360.*3600. */);
		alt = alt / Asec_Radian;
	}
}

moondata(tim)
long	tim;
{
	double	lst, beta, rm, sa, ca, sl, cl, sb, cb, x, y, z, l, m, n;

/* compute location of the moon */
/* Ephemeris elements from 1985 Almanac */

	timedata(tim);

	Lm= 218.32 + 481267.883*Tu
		+ 6.29 * sin((134.9 + 477198.85*Tu)*Degree_to_Radian)
		- 1.27 * sin((259.2 - 413335.38*Tu)*Degree_to_Radian)
		+ 0.66 * sin((235.7 + 890534.23*Tu)*Degree_to_Radian)
		+ 0.21 * sin((269.9 + 954397.70*Tu)*Degree_to_Radian)
		- 0.19 * sin((357.5 +  35999.05*Tu)*Degree_to_Radian)
		- 0.11 * sin((186.6 + 966404.05*Tu)*Degree_to_Radian);

	beta=	  5.13 * sin(( 93.3 + 483202.03*Tu)*Degree_to_Radian)
		+ 0.28 * sin((228.2 + 960400.87*Tu)*Degree_to_Radian)
		- 0.28 * sin((318.3 +   6003.18*Tu)*Degree_to_Radian)
		- 0.17 * sin((217.6 - 407332.20*Tu)*Degree_to_Radian);

	px= 0.9508
		+ 0.0518 * cos((134.9 + 477198.85*Tu)*Degree_to_Radian)
		+ 0.0095 * cos((259.2 - 413335.38*Tu)*Degree_to_Radian)
		+ 0.0078 * cos((235.7 + 890534.23*Tu)*Degree_to_Radian)
		+ 0.0028 * cos((269.9 + 954397.70*Tu)*Degree_to_Radian);

/*	SD= 0.2725 * px;	*/

	rm= 1.0 / sin(px * Degree_to_Radian);

	lst= (100.46 + 36000.77*Tu) * Degree_to_Radian
		+ ((tim % Sec_per_day) + Local) * Tsec_to_Radian;

/* form geocentric direction cosines */

	sl= sin(Lm * Degree_to_Radian);
	cl= cos(Lm * Degree_to_Radian);
	sb= sin(beta* Degree_to_Radian);
	cb= cos(beta * Degree_to_Radian);

	l= cb * cl;
	m= 0.9175 * cb * sl - 0.3978 * sb;
	n= 0.3978 * cb * sl + 0.9175 * sb;

/* R.A. and Dec of Moon, geocentric*/

	am= atan2(m, l);
	dm= asin(n);

/* topocentric rectangular coordinates */

	cd= cos(dm);
	sd= n;
	ca= cos(am);
	sa= sin(am);
	sl= sin(lst);
	cl= cos(lst);

	x= rm * cd *ca - cp * cl;
	y= rm * cd * sa - cp * sl;
	z= rm * sd - sp;

/* finally, topocentric Hour-Angle and Dec */

	am = lst - atan2(y, x);
	ca = cos(am);
	sa = sin(am);
	am = atan2(sa,ca);
	rm = sqrt(x*x + y*y + z*z);
	dm = asin(z/rm);
	px = asin(1.0 / rm);
	SD = 0.2725 * px;
}

timedata(tim)
long	tim;
{

/* compute seconds from 2000 Jan 1.5 UT (Ephemeris Epoch) */
/* the VAX Epoch is     1970 Jan 1.0 UT (Midnight on Jan 1) */

	Julian_Day = (tim/Sec_per_day) +
				(double)(tim % Sec_per_day)/Sec_per_day + J1970;
	MJD= Julian_Day -J2000;	/* Julian Days past Epoch */
	Tu = MJD/36525.;		/* Julian Centuries past Epoch */

/* compute Sidereal time */

	Ru= 24110.54841 + Tu * (8640184.812866
		+ Tu * (0.09304 - Tu * 6.2e-6));	/* seconds */
	GMST = (tim % Sec_per_day) + Sec_per_day + fmod(Ru, (double)Sec_per_day);
	LST  = GMST + Local;
}

/* time functions, for Regulus */
char *gmctime(t)	/* re-hack for VAX, since ctime gives local */
long *t;
{
	long t1;

	t1 = *t - localdst;		/* convert to local time */
	return(ctime(&t1));
}

char *localctime(t)
long *t;
{
	long t1;

	t1 = *t + localdst;		/* convert to local time */
	return(gmctime(&t1));
}

struct tm *Rlocaltime(t)
long *t;
{
	long t1;

	t1 = *t + localdst;		/* convert to local time */
	return(gmtime(&t1));
}

/* double precision modulus, put in range 0 <= result < m */
double fmod(x, m)
double	x, m;
{
	long i;

	i = fabs(x)/m;		/* compute integer part of x/m */
	if (x < 0)	return( x + (i+1)*m);
	else		return( x - i*m);
}
%%
