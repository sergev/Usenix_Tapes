/*
  Here are two routines, jday.c and jdate.c.
  They are translations from ALGOL in Collected Algorithms of CACM.
*/

/*
** Takes a date, and returns a Julian day. A Julian day is the number of
** days since some base date  (in the very distant past).
** Handy for getting date of x number of days after a given Julian date
** (use jdate to get that from the Gregorian date).
** Author: Robert G. Tantzen, translator: Nat Howard
** Translated from the algol original in Collected Algorithms of CACM
** (This and jdate are algorithm 199).
*/
long
jday(mon, day, year)
int mon, day, year;
{
	long m = mon, d = day, y = year;
	long c, ya, j;

	if(m > 2) m -= 3;
	else {
		m += 9;
		--y;
	}
	c = y/100L;
	ya = y - (100L * c);
	j = (146097L * c) /4L + (1461L * ya) / 4L + (153L * m + 2L)/5L + d + 1721119L;
	return(j);
}
