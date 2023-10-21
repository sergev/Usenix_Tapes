#include <sys/types.h>
#include <time.h>

/* Return 1 if `y' is a leap year, 0 otherwise.
 */

static int leap (y) int y; {
    y += 1900;
    if (y % 400 == 0)
	return (1);
    if (y % 100 == 0)
	return (0);
    return (y % 4 == 0);
}

/* Return the number of days between Jan 1, 1970 and the given
 * broken-down time.
 */

static int ndays (p) struct tm *p; {
    register n = p->tm_mday;
    register m, y;
    register char *md = "\37\34\37\36\37\36\37\37\36\37\36\37";

    for (y = 70; y < p->tm_year; ++y) {
	n += 365;
	if (leap (y)) ++n;
    }
    for (m = 0; m < p->tm_mon; ++m)
	n += md[m] + (m == 1 && leap (y));
    return (n);
}

/* Convert a broken-down time (such as returned by localtime())
 * back into a `time_t'.
 */

time_t tm_to_time (tp) struct tm *tp; {
    register int m1, m2;
    time_t t;
    struct tm otm;

    t = (ndays (tp) - 1) * 86400L + tp->tm_hour * 3600L
	+ tp->tm_min * 60 + tp->tm_sec;
    /*
     * Now the hard part -- correct for the time zone:
     */
    otm = *tp;
    tp = localtime (&t);
    m1 = tp->tm_hour * 60 + tp->tm_min;
    m2 = otm.tm_hour * 60 + otm.tm_min;
    t -= ((m1 - m2 + 720 + 1440) % 1440 - 720) * 60L;
    return (t);
}
