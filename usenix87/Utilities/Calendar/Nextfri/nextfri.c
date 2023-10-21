/**		nextfri.c		**/

/** Reads the current date and returns the next Friday's  **/
/** date in the string format MM/DD/19YY                  **/
/** If the current date is a Friday, the current date is  **/
/** returned.                                             **/

/** 1987, Marc Ries **/

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

char           *short_dayname[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

struct tm      *localtime();

#ifdef TEST
main()
{
char           *nextfri();
    printf("%s\n", nextfri());
}
#endif TEST

/*
 * Takes a date, and returns a Julian day. 
 */
long
jday(mon, day, year)
    int             mon, day, year;
{
long            m = mon, d = day, y = year;
long            c, ya, j;

    if (m > 2)
	m -= 3;
    else
    {
	m += 9;
	--y;
    }
    c = y / 100L;
    ya = y - (100L * c);
    j = (146097L * c) / 4L + (1461L * ya) / 4L + (153L * m + 2L) / 5L + d + 1721119L;
    return (j);
}

/*
 * Julian date converter. Takes a julian date (the number of days since some
 * distant epoch or other), and returns an int pointer to static space. ip[0]
 * = month; ip[1] = day of month; ip[2] = year (actual year, like 1977, not 77
 * unless it was  77 a.d.); ip[3] = day of week (0->Sunday to 6->Saturday)
 * These are Gregorian. Copied from Algorithm 199 in Collected algorithms of
 * the CACM Author: Robert G. Tantzen, Translator: Nat Howard 
 */
int            *
jdate(ret, j)
    int             ret[4];
long            j;
{
long            d, m, y;

    ret[3] = (j + 1L) % 7L;
    j -= 1721119L;
    y = (4L * j - 1L) / 146097L;
    j = 4L * j - 1L - 146097L * y;
    d = j / 4L;
    j = (4L * d + 3L) / 1461L;
    d = 4L * d + 3L - 1461L * j;
    d = (d + 4L) / 4L;
    m = (5L * d - 3L) / 153L;
    d = 5L * d - 3 - 153L * m;
    d = (d + 5L) / 5L;
    y = 100L * y + j;
    if (m < 10)
	m += 3;
    else
    {
	m -= 9;
	++y;
    }
    ret[0] = m;
    ret[1] = d;
    ret[2] = y;
    return (int *) (ret);
}

char           *
nextfri()
{
char            buffer[11], yrbuf[5], tempbuf[10];
int             mm, dd, yy, wd, ip[4];
long            julian;
time_t          now, time();
struct tm      *t;

    now = time((long *) 0);
    t = localtime(&now);

    wd = t->tm_wday;
    mm = t->tm_mon + 1;
    dd = t->tm_mday;
    (void) sprintf(yrbuf, "19%d", t->tm_year);
    (void) sscanf(yrbuf, "%d", &yy);
    (void) sprintf(tempbuf, "%s", short_dayname[t->tm_wday]);

    julian = jday(mm, dd, yy);
    (void) jdate(ip, julian);

    switch (wd)
    {
    case 0:			/* "Sunday," */
	julian = julian + 5;
	break;
    case 1:			/* "Monday" */
	julian = julian + 4;
	break;
    case 2:			/* Tuesday" */
	julian = julian + 3;
	break;
    case 3:			/* Wednesay" */
	julian = julian + 2;
	break;
    case 4:			/* Thursday" */
	julian = julian + 1;
	break;
    case 5:			/* Friday" */
	/*
	 * NOTE: We ONLY look for the 'next friday' IF it isn't already a
	 * Friday! 
	 */
	break;
    case 6:			/* Saturday" */
	julian = julian + 6;
	break;
    default:
	printf("Something is screwed up --\n");
    }
    (void) jdate(ip, julian);

    (void) sprintf(buffer, "%d/%d/%d", ip[0], ip[1], ip[2]);
    return ((char *) buffer);
}
