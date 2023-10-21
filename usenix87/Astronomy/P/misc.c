#include	"p.h"

/*
 *  Do everything necessary to keep everything
 *  up to date.
 */
gestalt()
	{
	tempus();			/* fugit */
	earthorbit();			/* don't loose it */
	sunorbit();			/* or this, either */
}

/*
 *  Keep the now structure up to date.
 */
tempus()
	{
	now.gmt = now.lt;		/* copy over local time */
	lttogmt(&now.gmt, HEREZONE);	/* generate GMT */
	now.gst = now.gmt;
	gmttogst(&now.gst);		/* generate GST */
	now.lst = now.gst;
	gsttolst(&now.lst, HERELONG);	/* generate LST */

	now.jd = julianday(&now.lt);
	/*
	 *  In case it's not there.
	 */
	now.lt.tm_wday = dayofwk(now.jd);
}

/*
 *  Go through the routines to convert LST to LT using
 *  the now structure.
 */
struct	tm *
lsttolt(tmp)
register struct	tm	*tmp;
	{
	static	struct	tm	tm;

	tm = now.lt;			/* use the now date */

	tm.tm_hour = tmp->tm_hour;
	tm.tm_min = tmp->tm_min;
	tm.tm_sec = tmp->tm_sec;

	lsttogst(&tm, HERELONG);
	gsttogmt(&tm);
	gmttolt(&tm, HEREZONE);
	return(&tm);
}

/*
 *  Count the number of days from the epoch, Jan. 0, 1980.
 *  Convert date to Julian day number, then subtract the bias for
 *  Jan. 0, 1980.  Note that the number of days could be negative.
 */
REAL
epoch80(tmp)
register struct	tm	*tmp;
	{
	REAL	e80;

	/*
	 *  First convert to Julian day.
	 */
	e80 = julianday(tmp);
	/*
	 *  Then subtract the bias for the Jan 0, 1980, epoch.
	 */
	e80 -= 2444238.5;

	return(e80);
}

/*
 *  Return the number of days in the year of interest.
 */
dysize(yr)
	{
	if ((yr % 4) == 0 && yr % 100 && (yr % 400) == 0)
		return(366);
	return(365);
}

/*
 *  Compute and return a floating remainder.
 */
REAL
flrem(a, b)
REAL	a, b;
	{
	REAL	c;
	long	d;

	c = a / b;
	d = (long) c;
	c -= (REAL) d;
	return(c * b);
}

/*
 *  Compute and return day of the month as a floating number.
 */
REAL
flday(tmp)
register struct	tm	*tmp;
	{
	REAL	day;

	day = hmstohr(tmp) / 24.0;
	day += (REAL) tmp->tm_mday;
	return(day);
}

/*
 *  Fix the floating argument day into appropriate fields
 *  of the time structure.
 */
fixday(tmp, day)
register struct	tm	*tmp;
REAL	day;
	{
	REAL	hours;

	tmp->tm_mday = (int) day;
	/*
	 *  Extract the fraction of day and convert to REAL hours.
	 */
	hours = flrem(day, 1.0) * 24.0;
	tmp->tm_hour = (int) hours;

	hrtohms(tmp, hours);
}

/*
 *  Compute arctan of two arguments resolving the ambiguity
 *  as shown in Fig. 10, pg. 45.
 */
REAL
atan2a(y, x)
REAL	y, x;
	{
	REAL	a, lo, hi;

	a = (REAL) atan(y/x);

	if (y >= 0.0 && x >= 0.0) {
		lo = 0.0; hi = 90.0;
	} else if (y < 0.0 && x >= 0.0) {
		lo = 270.0; hi = 360.0;
	} else if (y < 0.0 && x < 0.0) {
		lo = 180.0; hi = 270.0;
	} else {
		lo = 90.0; hi = 180.0;
	}
	/*
	 *  Go to degrees.
	 */
	a *= RAD;

	if (lo <= a && a < hi)
		return(a/RAD);

	if (a < lo)
		while (a < lo)
			a += 90.0;
	else
		while (a >= hi)
			a -= 90.0;

	return(a/RAD);
}

/*
 *  Compute an object's eccentric anomaly using Kepler's method.
 *  This method is valid for e's < 0.1.
 */
REAL
kepler(M, e)
REAL	M, e;
	{
	REAL	E, delta, ad;

	iter = 0;
	E = M;
	while (1) {
		if (iter++ > 100)		/* not converging */
			break;

		delta = E - e * (REAL) sin(E) - M;
		if (delta < 0.0)
			ad = -delta;
		else
			ad = delta;

		if (ad < 10e-6)
			break;

		E -= delta / (1.0 - e * (REAL) cos(E));
	}
	return(E);
}

/*
 *  Bring the argument angle in to the range 0 - 359.999 ...
 *  This routine works for negative angles.
 */
REAL
dcanon(angle)
REAL	angle;
	{
	if (angle > 0.0)
		while (angle >= 360.0)
			angle -= 360.0;
	else
		while (angle < 0.0)
			angle += 360.0;
	return(angle);
}

/*
 *  Make the time argument canonical.
 */
REAL
tcanon(tim)
REAL	tim;
	{
	if (tim > 0.0)
		while (tim >= 24.0)
			tim -= 24.0;
	else
		while (tim < 0.0)
			tim += 24.0;
	return(tim);
}
