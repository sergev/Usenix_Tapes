#include	"p.h"

/*
 *  Compute the day of the year.  Put it in the time structure.
 *  And return it.
 *  (Sect. 3; Pg. 6)
 */
dayofyr(tmp)
register struct	tm	*tmp;
	{
	register int	day, mon;

	day = 1;
	mon = tmp->tm_mon;
	/*
	 *  Leap year
	 */
	if (dysize(tmp->tm_year) == 366 && mon >= 3)
		day += 1;
	while(--mon)
		day += dmsize[mon-1];

	tmp->tm_yday = day + tmp->tm_mday;
	return(tmp->tm_yday);
}

/*
 *  (Sect. 4; Pg. 9)
 */
REAL
julianday(tmp)
register struct	tm	*tmp;
	{
	short	y, m;
	register long	A, B, C, D;
	REAL	day;

	y = tmp->tm_year;
	m = tmp->tm_mon;
	if (m <= 2) {
		y--;
		m += 12;
	}
	/*
	 *  Beware of 1582 Oct 15
	 */
	if (tmp->tm_year <= 1582 && tmp->tm_mon <= 10 && tmp->tm_mday <= 15)
		B = 0;
	else {
		A = y / 100;
		B = 2 - A + (A / 4);
	}
	C = (long) (365.25 * (REAL) y);
	D = (long) (30.6001 * ((REAL) m + 1.0));
	/*
	 *  Get the floating equivalent of the day of the month.
	 */
	day = flday(tmp);
	day += 1720994.5;
	day += (REAL) B + (REAL) C + (REAL) D;

	return(day);
}

/*
 *  (Sect. 5; Pg. 11)
 */
struct	tm *
jdtodate(jday)
REAL	jday;
	{
	static	struct	tm	jtm;
	REAL	F, d;
	register long	A, B, C, D, E, G, H, I;

	jday += 0.5;
	I = (long) jday;
	F = flrem(jday, 1.0);
	if (I > 2299160) {
		A = (long) (((REAL) I - 1867216.25) / 36524.25);
		B = I + 1 + A - (A / 4);
	} else
		B = I;		/* note the misprint in the book */

	C = B + 1524;
	D = (long) (((REAL) C - 122.1) / 365.25);
	E = (long) (365.25 * (REAL) D);
	G = (long) (((REAL) C - (REAL) E) / 30.6001);
	H = (long) ((REAL) G * 30.6001);
	d = (REAL) C - (REAL) E + F - H;

	if (G <= 13)
		jtm.tm_mon = G - 1;
	else
		jtm.tm_mon = G - 13;

	if (jtm.tm_mon <= 2)
		jtm.tm_year = D - 4715;
	else
		jtm.tm_year = D - 4716;

	fixday(&jtm, d);
	/*
	 *  Compute the day of the year into the structure.
	 */
	dayofyr(&jtm);
	/*
	 *  Also determine the day of the week.
	 */
	jtm.tm_wday = dayofwk(jday);
	/*
	 *  Possible adjust for DST.
	 */
	dstime(&jtm);

	return(&jtm);
}

/*
 *  (Sect. 6; Pg. 12)
 */
dayofwk(jday)
REAL	jday;
	{
	REAL	A;
	register int	wkday;

	A = (jday + 1.5) / 7.0;
	wkday = (int) (flrem(A, 1.0) * 7.0);
	return(wkday);
}

/*
 *  (Sect. 7; Pg. 13)
 */
REAL
hmstohr(tmp)
register struct	tm	*tmp;
	{
	REAL	sec, min, hr;

	sec = (REAL) tmp->tm_sec;
	min = (REAL) tmp->tm_min;
	hr = (REAL) tmp->tm_hour;

	sec /= 60.0;
	min += sec;
	min /= 60.0;
	hr += min;
	return(hr);
}

/*
 *  (Sect. 7; Pg. 13)
 */
REAL
dmstodeg(degp)
register struct	deg	*degp;
	{
	REAL	sec, min, hr;

	sec = (REAL) degp->sec;
	min = (REAL) degp->min;
	hr = (REAL) degp->deg;

	sec /= 60.0;
	min += sec;
	min /= 60.0;
	hr += min;
	return(hr);
}

/*
 *  (Sect. 8; Pg. 14)
 */
hrtohms(tmp, hours)
register struct	tm	*tmp;
REAL	hours;
	{
	REAL	F;

	tmp->tm_hour = (int) hours;

	F = flrem(hours, 1.0);
	F *= 60.0;
	tmp->tm_min = (int) F;

	F = flrem(F, 1.0);
	F *= 60.0;
	tmp->tm_sec = (int) F;
}

/*
 *  (Sect. 8; Pg. 14)
 */
degtodms(degp, hours)
register struct	deg	*degp;
REAL	hours;
	{
	REAL	F;

	if (hours < 0.0) {
		hours = -hours;
		degp->sg = '-';
	} else
		degp->sg = ' ';

	degp->deg = (int) hours;

	F = flrem(hours, 1.0);
	F *= 60.0;
	degp->min = (int) F;

	F = flrem(F, 1.0);
	F *= 60.0;
	degp->sec = (int) F;
}

/*
 *  (Sect. 9; Pg. 16)
 */
lttogmt(tmp, zone)
register struct	tm	*tmp;
REAL	zone;
	{
	REAL	dectime;

	dectime = hmstohr(tmp);
	/*
	 *  Correct for DST, if necessary.  Use the dst flag in the
	 *  now structure.
	 */
	if (now.lt.tm_isdst > 0)
		zone -= 1.0;
	dectime += zone;

	dectime = tcanon(dectime);

	hrtohms(tmp, dectime);
	tmp->tm_isdst = GMT;
}

/*
 *  (Sect. 10; Pg. 18)
 */
gmttolt(tmp, zone)
register struct	tm	*tmp;
REAL	zone;
	{
	REAL	dectime;

	dectime = hmstohr(tmp);
	/*
	 *  Correct for DST, if necessary.  Use the dst flag in the
	 *  now structure.
	 */
	if (now.lt.tm_isdst > 0) {
		zone -= 1.0;
		tmp->tm_isdst = DST;
	} else
		tmp->tm_isdst = LT;

	dectime -= zone;

	dectime = tcanon(dectime);

	hrtohms(tmp, dectime);
}

/*
 *  (Sect. 12; Pg. 20)
 */
#define	S12A	0.0657098
#define	S12C	1.002738
#define	S12D	0.997270

gmttogst(tmp)
register struct	tm	*tmp;
	{
	REAL	ydays, gmthours;
	REAL	T0, B;

	ydays = (REAL) dayofyr(tmp);
	ydays *= S12A;

	B = s12b(tmp);
	T0 = ydays - B;

	gmthours = hmstohr(tmp) * S12C;
	T0 += gmthours;
	T0 = tcanon(T0);

	hrtohms(tmp, T0);

	tmp->tm_isdst = GST;
}

/*
 *  Compute B in section 12.
 */
REAL
s12b(tmp)
register struct	tm	*tmp;
	{
	REAL	B, JD, R, S, T, U;
	static	struct	tm	jan0;

	jan0.tm_year = tmp->tm_year;
	jan0.tm_mon = 1;		/* January 0 */
	JD = julianday(&jan0);
	S = JD - 2415020.0;
	T = S / 36525.0;
	R = 6.6460656 + (2400.051262 * T) + (0.00002581 * T * T);
	U = R - (24.0 * (REAL) (tmp->tm_year - 1900));
	B = 24.0 - U;
	return(B);
}

/*
 *  (Sect. 13; Pg. 22)
 */
gsttogmt(tmp)
register struct	tm	*tmp;
	{
	REAL	gmthours, gsthours, ydays, T0, B;

	ydays = (REAL) dayofyr(tmp);
	ydays *= S12A;

	B = s12b(tmp);
	T0 = ydays - B;
	T0 = tcanon(T0);

	gsthours = hmstohr(tmp);
	gsthours -= T0;

	gsthours = tcanon(gsthours);
	gmthours = gsthours * S12D;

	hrtohms(tmp, gmthours);

	tmp->tm_isdst = GMT;
}

/*
 *  (Sect. 14; Pg. 24)
 */
gsttolst(tmp, longitude)
register struct	tm	*tmp;
REAL	longitude;
	{
	REAL	dectime, diff;

	dectime = hmstohr(tmp);
	diff = longitude / 15.0;
	dectime -= diff;	/* W long is positive, W is negative */
	dectime = tcanon(dectime);
	hrtohms(tmp, dectime);
	tmp->tm_isdst = LST;
}

/*
 *  (Sect. 15; Pg. 25)
 */
lsttogst(tmp, longitude)
register struct	tm	*tmp;
REAL	longitude;
	{
	REAL	dectime, diff;

	dectime = hmstohr(tmp);
	diff = longitude / 15.0;
	dectime += diff;	/* W long is positive, W is negative */
	dectime = tcanon(dectime);
	hrtohms(tmp, dectime);
	tmp->tm_isdst = GST;
}

/*
 *  Update the local time in the now structure.  Or sleep the equivalent
 *  amount if we're running in real time.
 */
ticktock()
	{
	short	tpast, tickint, ticktime;
	long	cursec;
	REAL	jday, dt;
	double	fabs();

	/*
	 *  Sleep the interval away.
	 */
	if (eltime) {
		/*
		 * Convert julian day to seconds.
		 */
		jday = julianday(&now.lt) * 1440.0 * 60.0;
		/*
		 * Find the number of seconds into the day.
		 */
		jday = flrem(jday, 1440.0 * 60.0);
		cursec = (long) (jday + 0.5);

		tickint = (unsigned) ((REAL) fabs(deltat) * 1440.0 * 60.0);
		tpast = cursec % tickint;
		ticktime = tickint - tpast;
		/*
		 * Beware!  We don't have one second resolution.
		 */
		if (ticktime == 1)
			ticktime += tickint;
		/*
		 * Keep the REAL equivalent of the ticktime in deltat
		 * units.
		 */
		dt = ((REAL) ticktime) / (1440.0 * 60.0);
		/*
		 *  Break up the sleep to look for input.
		 */
		while (ticktime > 0) {
			move(pt[SEPMAT].row+7, SECCOL);
			printw("(%d sec)  ", ticktime);
			move(0, 0); refresh();

			if (ticktime > 5) {
				sleep(5);
				ticktime -= 5;
			} else {
				sleep(ticktime);
				break;
			}
			scan();
			/*
			 *  If a request was processed, break out of the
			 *  sleep loop; this request may affect sleep time.
			 */
			if (dorequest() >= 0)
				break;
		}
	} else
		dt = deltat;

	jday = julianday(&now.lt) + dt;
	/*
	 *  Copy back the entire structure.
	 */
	now.lt = *jdtodate(jday);
}

/*
 *  Generate a new localtime for the now structure.
 */
settime()
	{
	struct	tm	*localtime();
	long	tvec;

	time(&tvec);
	now.lt = *localtime(&tvec);
	now.lt.tm_mon++;
	now.lt.tm_yday++;
	now.lt.tm_year += 1900;
}

/*
 * The following table is used for 1974 and 1975 and
 * gives the day number of the first day after the Sunday of the
 * change.
 */
static struct {
	int	daylb;
	int	dayle;
} daytab[] = {
	5,	333,	/* 1974: Jan 6 - last Sun. in Nov */
	58,	303,	/* 1975: Last Sun. in Feb - last Sun in Oct */
};

/*
 *  Determine if the time pointed to by tmp should be set to DST.
 *  If so, do it.
 */
dstime(tmp)
register struct	tm	*tmp;
	{
	register int dayno, daylbegin, daylend;

	/*
	 *  Don't try to convert to DST for times and dates earlier
	 *  than 1/1/1900.
	 */
	if (tmp->tm_year < 1900)
		return;

	dayno = tmp->tm_yday;
	daylbegin = 119;	/* last Sun in Apr */
	daylend = 303;		/* Last Sun in Oct */
	if(tmp->tm_year == 1974 || tmp->tm_year == 1975) {
		daylbegin = daytab[tmp->tm_year-1974].daylb;
		daylend = daytab[tmp->tm_year-1974].dayle;
	}
	daylbegin = sunday(tmp, daylbegin);
	daylend = sunday(tmp, daylend);
	if(daylight &&
	    (dayno > daylbegin || (dayno == daylbegin && tmp->tm_hour >= 2)) &&
	    (dayno < daylend || (dayno == daylend && tmp->tm_hour < 1))) {
		tmp->tm_isdst = DST;
	} else
		tmp->tm_isdst = LT;
}

/*
 *  The argument is a 0-origin day number.
 *  The value is the day number of the first
 *  Sunday on or after the day.
 */
static int
sunday(t, d)
register struct tm *t;
register int d;
	{
	if(d >= 58)
		d += dysize(t->tm_year) - 365;
	return(d - (d - t->tm_yday + t->tm_wday + 700) % 7);
}
