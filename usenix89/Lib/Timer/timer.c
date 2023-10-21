/*
 * "timer.c", zie timer(3).
 */

#include	<sys/types.h>
#include	<sys/timeb.h>
#include	"timer.h"

/*
 * Geef huidige tijd in sekonden.
 * Deze fct. is implemetatie-afhankelijk van ftime(2).
 */
static double sec_time ()
{
	struct timeb ft;

	ftime (&ft);
	return (double) ft.time + (double) ft.millitm / (double) 1000;
}

/*
 * Geef TIMER met timeout na sec sekonden (mag < 0 zijn).
 * Cleart als sec == NO_TIME.
 */
TIMER timer (sec)
double sec;
{
	if (sec == NO_TIME)	/* ge-clear-de timer */
		return (TIMER) NO_TIME;
	else
		return (TIMER) (sec_time () + sec);
}

/*
 * Geef # sekonden tot timeout van t (kan negatief zijn).
 * Geeft NO_TIME voor ge-clear-de TIMER.
 */
double time_left (t)
TIMER t;
{
	if (t == NO_TIME)
		return NO_TIME;
	else
		return (double) t - sec_time ();
}

/*
 * Set tvec [0] .. tvec [n] op timeout na sec sekonden.
 * Als sec is NO_TIME, worden alle tvec [i] afgezet.
 */
set_tvec (tvec, n, sec)
TIMER *tvec;
unsigned n;
double sec;
{
	if (sec != NO_TIME)
		sec += sec_time ();

	while (n-- != 0)
		tvec [n] = (TIMER) sec;
}

/*
 * Geef index van eerste timer met een timeout in circular search
 * si .. n - 1, 0 .. si - 1.
 * Levert n als si >= n, of als geen timeout is opgetreden.
 */
unsigned tvec_timeout (tvec, n, si)
TIMER *tvec;
unsigned n, si;
{
	double now;
	unsigned i;

	if (si >= n)
		return n;

	now = sec_time ();

	for (i = si; i < n; i++)
		if ((double) tvec [i] <= now)
			return i;
	for (i = 0; i < si; i++)
		if ((double) tvec [i] <= now)
			return i;

	return n;
}

/*
 * Geef index in tvec van de (eerste) oudste timer.
 * Levert n als alle timers afstaan.
 */
unsigned tvec_oldest (tvec, n)
TIMER *tvec;
unsigned n;
{
	unsigned i, oldest = 0;

	for (i = 1; i < n; i++)
		if (tvec [i] < tvec [oldest])
			oldest = i;

	if (n != 0 && tvec [oldest] != (TIMER) NO_TIME)
		return oldest;
	else
		return n;
}
