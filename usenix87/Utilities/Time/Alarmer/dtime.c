/* dtime.c - routines to do ``ARPA-style'' time structures */

#include "tws.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>
#ifndef	BSD42
#include <time.h>
#else	BSD42
#include <sys/time.h>
#endif	BSD42

/*  */

#define	abs(a)	(a >= 0 ? a : -a)

/*  */

static char *month[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char *day[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

/*  */

struct keywd {
    char   *key;
    int     value;
};

#define	NZONES	(sizeof (zones)/sizeof (struct keywd))

static struct keywd zones[] = {	/* order sensitive */
    "UT", 0,
    "EST", -5, "CST", -6,  "MST", -7, "PST", -8, "PDT", -9,
    "A",   -1, "B",   -2,  "C",   -3, "D",   -4,
    "E",   -5, "F",   -6,  "G",   -7, "H",   -8,
    "I",   -9, "K",  -10,  "L",  -11, "M",  -12,
    "N",    1, "O",    2,  "P",    3, "Q",    4,
    "R",    5, "S",    6,  "T",    7, "U",    8,
    "V",    9, "W",   10,  "X",   11, "Y",   12
};


long    time ();
struct tm *localtime ();

/*  */

char   *dtime (clock)
long   *clock;
{
    return dasctime (dlocaltime (clock));
}


char *dtimenow()
{
    long    clock;

    time (&clock);
    return dtime (&clock);
}


char   *dctime (tw)
struct tws *tw;
{
    static char buffer[25];

    if (!tw)
	return NULL;

    sprintf (buffer, "%.3s %.3s %02d %02d:%02d:%02d %.4d\n",
	    day[tw -> tw_wday], month[tw -> tw_mon], tw -> tw_mday,
	    tw -> tw_hour, tw -> tw_min, tw -> tw_sec,
	    tw -> tw_year >= 100 ? tw -> tw_year : 1900 + tw -> tw_year);

    return buffer;
}

/*  */

struct tws *dlocaltime (clock)
long   *clock;
{
    struct tm  *tm;
    struct timeb    tb;
    static struct tws   tw;

    if (!clock)
	return NULL;

    tm = localtime (clock);
    tw.tw_sec = tm -> tm_sec;
    tw.tw_min = tm -> tm_min;
    tw.tw_hour = tm -> tm_hour;
    tw.tw_mday = tm -> tm_mday;
    tw.tw_mon = tm -> tm_mon;
    tw.tw_year = tm -> tm_year;
    tw.tw_wday = tm -> tm_wday;
    tw.tw_yday = tm -> tm_yday;
    tw.tw_isdst = tm -> tm_isdst;

    ftime (&tb);
    tw.tw_zone = -tb.timezone;
    tw.tw_sday = 0;

    return (&tw);
}

/*  */

char   *dasctime (tw)
struct tws *tw;
{
    static char buffer[80],
                result[80];

    if (!tw)
	return NULL;

    sprintf (buffer, "%02d %s %02d %02d:%02d:%02d %s",
	    tw -> tw_mday, month[tw -> tw_mon], tw -> tw_year,
	    tw -> tw_hour, tw -> tw_min, tw -> tw_sec,
	    dtimezone (tw -> tw_zone, tw -> tw_isdst));

    if (tw -> tw_sday > 0)
	sprintf (result, "%s, %s", day[tw -> tw_wday], buffer);
    else
	if (tw -> tw_sday < 0)
	    strcpy (result, buffer);
	else
/*	    sprintf (result, "%s (%s)", buffer, day[tw -> tw_wday]);*/
	    sprintf (result, "%s, %s", day[tw -> tw_wday], buffer);
    return result;
}

/*  */

char   *dtimezone (zone, dst)
int     zone,
        dst;
{
    int     i,
            j,
            hours,
            mins;
    static char buffer[10];

    if (zone < 0) {
	mins = -((-zone) % 60);
	hours = -((-zone) / 60);
    }
    else {
	mins = zone % 60;
	hours = zone / 60;
    }
#ifndef MASSCOMP
    if (dst)
	hours -= 1;
#endif
    if (mins == 0)
	for (i = 0, j = NZONES; i < j; i++)
	    if (zones[i].value == hours) {
		strcpy (buffer, zones[i].key);
		if (dst && buffer[1] == 'S')
		    buffer[1] = 'D';
		return buffer;
	    }

    sprintf (buffer, "%s%02d%02d",
	    zone < 0 ? "-" : "+", abs (hours), abs (mins));
    return buffer;
}
