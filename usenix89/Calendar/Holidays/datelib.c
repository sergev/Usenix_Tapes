

/*            DATELIB -- LIBRARY FUNCTIONS FOR UNIX DATES         */
/*                                                                */
/*   Copyright (c) 1987 Tigertail Associates. All Rights Reserved */
/*                      320 North Bundy Drive                     */
/*                      Los Angeles, California 90049             */
/*                      USA                                       */
/*                                                                */
/*   e-mail: (uucp) ucla-cs!buz   or (arpa) buz@cs.ucla.edu       */
/*                                                                */

#include <time.h>
#include "taxc.h"

#define 	DAY		(24L*3600L)
#define 	YEAR	(DAY*365L)         /* in seconds */
#define 	RYEAR	31556926L          /* 365.2422 * DAY */
#define 	NERA 	(56*YEAR+14*DAY)   /* 56 years and 14 leap days */

#ifdef BSD
#define     TZONE   ((TZV)*3600)       /* in seconds west of greenwich */
long time();
struct tm *localtime();
#else
extern long timezone;
extern Bool daylight;
#endif

/* This library provides the following functions */
#if defined(lint) || defined(BSD) || defined(SYSV)
long   uxdate(), uxldate(), uxtime(), uxeaster();
int    yrday(),  weeknum(), daynum();
Bool   leap();
void   datex(),  ldatex(),  timex();
#else
long   uxdate(int,int,int), uxldate(int,int,int), uxtime(int,int,int);
long   uxeaster(int);
int    yrday(int,int,int), weeknum(long), daynum(long);
Bool   leap(int);
void   datex(long,int*,int*,int*,int*,int*);
void   ldatex(long,int*,int*,int*,int*,int*);
void   timex(long,int*,int*,int*);
#endif

/* This external array is sometimes handy */
int monthdays[] = {   0,  31,  59,  90, 120, 151, 181,
	                212, 243, 273, 304, 334, 365       };

/*     Function uxdate
 *     Return the number of seconds between Jan 1, 1970 and the 
 *     0h 0m 0s of the date pm/pd/py in the current unix era.
 *     The date can be viewed as Greenwich date or it can be viewed
 *     as an abstract date, if you need to combine it with time it
 *     needs correction for the timezone and daylight savings time.
 *     The year is the full christian year, 70 is nonsense (and not
 *     in the current era anyway, it is not interpreted as the year
 *     1970.  Calculation is only done for the Gregorian calendar.
 *
 *     A unix date is assumed to be signed and represent dates before
 *     1970 -- this is contrary to some speculation.  Using this scheme
 *     unix dates cover an era from about 1903 to about 2032.  Beyond this
 *     other reperesentations need to be used.  The alternative is to
 *     consider unix dates as unsigned longs.  Using that scheme no date
 *     before 1970 can be represented; these programs use signed longs
 *     with a beginning era of 1914.
 */

long PROC uxdate ( py, pm, pd )
int py, pm, pd;

{   long yrs = (py-1970L);   /* Number of years from 0h 1970  */
	long nd  = yrs * 365L;   /* Number of days in those years */

	if ( yrs >= 0 )          /* Add in bygone leap days       */
	then nd  += ((yrs+1L) / 4L);
	else nd  += ((yrs-2L) / 4L);

    if ( pm > 2 )           /* Leap year correction for this year */
	then nd += leap(py);

	     /*  ( base + prior months + this month ) * sec/day */
    return ( ( nd + monthdays[pm-1] + pd - 1 ) * DAY );
}

    

/*     Function uxldate
 *     Return the number of seconds between GMT Jan 1, 1970 0h 0m 0s
 *     and local time 0h 0m 0s of date pm/pd/py in the current era.
 *     The date is thus based on local time. The year is the full year,
 *     `70' is nonsense not the year `1970'.  Calculation is Gregorian
 *     calendar for the current era.
 */

long PROC uxldate ( py, pm, pd)
int py, pm, pd;

{   long n;
#ifdef BSD
    struct tm *tp;
	tp = localtime();
	n  = uxdate ( py, pm, pd ) + (TZONE - (tp->tm_isdst ? 3600L : 0L));
#else
	tzset();
	n  = uxdate ( py, pm, pd ) + (timezone - (daylight ? 3600L : 0L));
#endif
    return (n);
}



/*  Routine datex (date extraction).  The opposite function of uxdate;
 *  that is, break down a Unix date into:
 *     cy     current christian year            1903 .. 2036
 *     cm     number full months so far this year  0 ..   11 where 0 = Jan
 *     cyd    number full days   so far this year  0 ..  365 where 0 = Jan 1st
 *     cmd    number full days   so far this month 0 ..   30 where 0 = 1st
 *     cwd    number full days   so far this week  0 ..    6 where 0 = Sun
 *  This provides almost the same functions as the system provided gmtime
 *  without static storage pointers (considered dirty).
 *  See timex for time extraction.
 */

void PROC datex ( udate, cy, cm, cyd, cmd, cwd )
long udate;
int  *cy, *cm, *cyd, *cmd, *cwd;

{   long xd;          /* modified date -- copy to muck around with */
	long csy;         /* current start of year in seconds */
    int  nld;         /* number of leap days to start of year */
    long nlds;        /* number of leap days to start of year in seconds */
    int  xy;          /* x number of full years since 1970 */
	int  yd, md, wd;  /* day of year, month, and week */
	int  am, rm;      /* approx. month, real month */
	int  tld=0;       /* temporary leap day correction in days */
	Bool tly;         /* is leap day */
	int  era;         /* starting year of date sequence */

	if ( udate < 0 )
	then
	{    era = 1914;                /* Calendar & LY orientation == 1970 */
		 xd  = NERA + udate; }
	else
	{    era = 1970;
		 xd  = udate;        }

	nld  = (xd+YEAR+306*DAY)/(4L*YEAR+DAY); /* leap days in era incl. now */
	nlds = nld * DAY;
	xy   = (xd-nlds) / YEAR;        /* Years from origin (yrs) */
	*cy  = era + xy;                /* Return year */

	csy  = xy*YEAR + ((xy+1)/4) * DAY; /* Start of this year in sec */
	yd   = ((xd - csy) / DAY);      /* Full days so far this year in days */
	/* printf ( "csy=%ld, yd=%d, cy=%d, ", csy/DAY, yd, *cy ); */
	if ( tly = leap(*cy) )          /* If this year a leap year */
	then tld = (yd>=60) ? 1 : 0;    /* Adjust for it; 2 makes Mar work */
	else tld = 0;

	/* printf ( "nld=%d, xy=%d, xd=%ld, ", nld, xy, xd/DAY ); */
	/* printf ( "tld=%d, csy=%ld, yd=%d\n", tld, csy/DAY, yd ); */

	*cyd = yd;	                    /* Return full days so far this year */
	am = (yd-tld) / 29;	            /* Approximate current month  0-12 */
	if ( (yd-tld) < monthdays[am] ) /* Determine real month       0-11 */
	then rm = am - 1;
	else rm = am;
	if (tly&&(yd==59)) then rm = 1; /* Fix leap day (Ugh) */
	*cm  = rm;					    /* Return full months in year so far */

	md   = yd - monthdays[rm] - tld;/* Full days in month so far */
	*cmd = md;                      /* Return full days in month so far */

	wd = ( 4L + ( xd / DAY )) % 7;  /* in days */
	*cwd = wd;                      /* Return full days in week so far */

	return;
}



/*  Routine ldatex (date extraction).  The opposite function of uxldate;
 *  that is, break down a local time Unix date into the same things as datex.
 */

void PROC ldatex ( udate, cy, cm, cyd, cmd, cwd )
long udate;
int  *cy, *cm, *cyd, *cmd, *cwd;

{   long ud;
#ifdef BSD
    struct tm *tp;
	tp = localtime();
	ud = udate - (TZONE - (tp->tm_isdst ? 3600L : 0L));
	datex( ud, cy, cm, cyd, cmd, cwd );
#else
	tzset();
	ud = udate - (timezone - (daylight ? 3600L : 0L));
	datex( ud, cy, cm, cyd, cmd, cwd );
#endif
	return;
}


/*     Function uxtime
 *     Convert a broken-down local time ph:pm:ps to
 *     seconds since local midnight.
 */


long PROC uxtime ( ph, pm, ps )
int ph, pm, ps;

{   return ( ph * 3600L + pm * 60L + ps );
}




/*     Routine timex.  Do the opposite of uxtime take a Unix time
 *     and return:
 *         ch    number of full hours since midnight
 *         cm    number of full minutes since last hour
 *         cs    number of full seconds since last minute
 */

void PROC timex ( udate, ch, cm, cs )
long udate;
int *ch, *cm, *cs;

{   long ct = (udate<0?NERA+udate:udate); /* Make date/time positive */
    ct     %= DAY;                        /* get rid of the days part */
	*ch     = ct / 3600L;                 /* get the hours out */
	*cm     = (int)(ct-(*ch)*3600L) / 60; /* get the minutes out */
	*cs     = ct % 60;                    /* get the seconds out */
	return;
}



/*     Function uxeaster
 *     Given a year (eg 1984) it provides a Unix date
 *     for 0h Easter of that year.
 */


long PROC uxeaster ( py )
int py;

{   /* The following amusing algorithm comes from page 5 of
     * <Practical Astronomy with your calculator>, second edition,
	 * by Peter Duffett-Smith, Cambridge University Press, 1979, 1981.
	 * The truth of the algorithm is self evident.
	 */

    int  a = py % 19;
	int  b = py / 100;
	int  c = py % 100;
	int  d = b / 4;
	int  e = b % 4;
	int  f = (b + 8) / 25;
	int  g = (b - f + 1) / 3;
	int  h = (19*a + b - d - g + 15) % 30;
	int  i = c / 4;
	int  k = c % 4;
	int  l = (32 + 2*e + 2*i  - h - k) % 7;
	int  m = (a + 11*h + 22*l) / 451;
	int  t = (h + l - 7*m + 114);
	int  n = t / 31;
	int  p = t % 31;

	return ( uxdate( py, n, p+1 ) );
}


/*
 *  Function yrday
 *  Gives the number of full days so far this year for a given date.
 *  Returns an int.  Excludes today so Jan 1st = 0;
 */

int PROC yrday ( y, m, d )
int y, m, d;

{   return ( monthdays[m-1] + (m>2?leap(y):0) + d-1);
         /*  days to month  +   leap day      +  month day */
}



/*   Function leap
 *   Return 1 if `y' is a leap year, 0 otherwise.
 */

int PROC leap (y)
int y;

{   if (y % 400 == 0) then return (1);
    if (y % 100 == 0) then return (0);
    return ( ((y % 4 == 0) ? 1 : 0) );
}



/*     Function weeknum
 *     Return the number of full Saturdays so far this month (0-5).
 */

int PROC weeknum ( udate )
long udate;

{   int  y, m, dy, dm, dw;      /* breakdown of udate from datex */

	datex ( udate, &y, &m, &dy, &dm, &dw );
    return ( (dm + (dw - dm + 35) % 7) / 7);
}



/*     Function daynum
 *     Return the number of full days that are the same weekday so far
 *     this month (0-4).  e.g. If date is the 3rd Tuesday of Sept. ...
 *     Function returns 2;
 */

int PROC daynum ( udate )
long udate;

{   int  y, m, dy, dm, dw;      /* breakdown of udate from datex */

	datex ( udate, &y, &m, &dy, &dm, &dw );
    return ( dm / 7 );
}
