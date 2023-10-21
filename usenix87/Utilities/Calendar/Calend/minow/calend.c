/*
 * 			C A L E N D A R
 *
 * Usage:
 *	calend	MM		If small, it's a month, if large, a year.
 * or
 *	calend	YYYY MM		year/month
 * or
 *	calend	MM YYYY	
 */

/*)BUILD
*/

#ifdef DOCUMENTATION

title	calend	Print a Calendar
index	calend	Print a Calendar

usage

	calend [year] [month]

description

	When invoked without arguments, calend prints a
	calendar for the preceeding, current, and next
	months of the current year.

	If a month is given (a value from 1 through 12),
	it prints the three months centered on the requested
	month.  For example,

	    calend 12

	Prints November and December for this year, and
	January for next year.

	If a year is given, it prints a calendar for the
	entire year:

	    calend 1985

	If both values are given, it prints the three months
	centered on the indicated date:

	    calend 1752 9
	    calend 9 1752

	Note that a three or four digit number will always be taken as
	a year.  A one or two digit number will be either a month
	or a year in the 20th century:

	    calend 78		(equals calend 1978)
	    calend 0078		(early common era)

bugs

	The change from the Julian to Gregorian calendars follows
	usage in England and her colonies.  Options should be provided
	to process the change for other countries (and localities).
	This is, however, a fairly complex task with little payback.

	The year didn't always start in January.

references

	Enclycopaedia Brittanica, 13th edition, Vol. 4, p. 987 et. seq.

	Parise, Frank, ed., The Book of Calendars, Facts on File,
	New York, 1982.

author

	Martin Minow

#endif
/*
 * Edit history
 * 1978?	??	Original version for Decus C
 * 27-Dec-1985	MM	Usenet submission.
 * 10-Jan-1986	DWV	Bug in 1752 magic
 */

#include <stdio.h>
#include <time.h>
#ifdef	decus
int	$$narg = 1;			/* Don't prompt			*/
#endif
#ifdef vms
#include		ssdef
#define	IO_ERROR	SS$_ABORT
#define	IO_SUCCESS	SS$_NORMAL
#endif
#ifndef	IO_ERROR
#define	IO_SUCCESS	0		/* Unix definitions		*/
#define	IO_ERROR	1
#endif
#define	EOS	0

#define	ENTRY_SIZE	3		/* 3 bytes per value		*/
#define DAYS_PER_WEEK	7		/* Sunday, etc.			*/
#define	WEEKS_PER_MONTH	6		/* Max. weeks in a month	*/
#define	MONTHS_PER_LINE	3		/* Three months across		*/
#define	MONTH_SPACE	3		/* Between each month		*/

/*
 * calendar() stuffs data into layout[],
 * output() copies from layout[] to outline[], (then trims blanks).
 */
char	layout[MONTHS_PER_LINE][WEEKS_PER_MONTH][DAYS_PER_WEEK][ENTRY_SIZE];
char	outline[(MONTHS_PER_LINE * DAYS_PER_WEEK * ENTRY_SIZE)
	    + (MONTHS_PER_LINE * MONTH_SPACE)
	    + 1];

char	*weekday = " S  M Tu  W Th  F  S";
char	*monthname[] = {
	"???",						/* No month 0	*/
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

main(argc, argv)
int		argc;
char		*argv[];
{
	register int	month;
	register int	year;

	register int	arg1val;
	int		arg1len;
	int		arg2val;
	int		tvec[2];
	struct tm	*tm;

	time(&tvec[0]);
	tm = localtime(&tvec[0]);
	year = tm->tm_year + 1900;
	month = tm->tm_mon + 1;
	if (argc <= 1) {
	    /*
	     * No arguments mean do last, this, and next month
	     */
	    do3months(year, month);
	}
	else {
	    arg1val = atoi(argv[1]);
	    arg1len = strlen(argv[1]);
	    if (argc == 2) {
		/*
		 * Only one argument, if small, it's a month.  If
		 * large, it's a year.  Note:
		 *	calend	0082	Year '82
		 *	calend	82	Year 1982
		 */
		if (arg1len <= 2 && arg1val <= 12)
		    do3months(year, arg1val);
		else {
		    if (arg1len <= 2 && arg1val > 0 && arg1val <= 99)
			arg1val += 1900;
		    doyear(arg1val);
		}
	    }
	    else {
		/*
		 * Two arguments, allow 1980 12 or 12 1980
		 */
		arg2val = atoi(argv[2]);
		if (arg1len > 2)
		    do3months(arg1val, arg2val);
		else
		    do3months(arg2val, arg1val);
	    }
	}
	exit(IO_SUCCESS);
}

doyear(year)
int		year;
/*
 * Print the calendar for an entire year.
 */
{
	register int	month;

	if (year < 1 || year > 9999)
	    usage("year", year);
	printf("\n\n\n%35d\n\n", year);
	for (month = 1; month <= 12; month += MONTHS_PER_LINE) {
	    printf("%12s%23s%23s\n",
		    monthname[month],
		    monthname[month+1],
		    monthname[month+2]);
	    printf("%s   %s   %s\n", weekday, weekday, weekday);
	    calendar(year, month+0, 0);
	    calendar(year, month+1, 1);
	    calendar(year, month+2, 2);
	    output(3);
#if MONTHS_PER_LINE != 3
	    << error, the above won't work >>
#endif
	}
	printf("\n\n\n");
}

domonth(year, month)
int		year;
int		month;
/*
 * Do one specific month -- note: no longer used
 */
{
	if (year < 1 || year > 9999)
	    usage("year", year);
	if (month <= 0 || month > 12)
	    usage("month", month);
	printf("%9s%5d\n\n%s\n", monthname[month], year, weekday);
	calendar(year, month, 0);
	output(1);
	printf("\n\n");
}

do3months(thisyear, thismonth)
int		thisyear;
register int	thismonth;
/*
 * Do last month, this month, and next month.  The parameters
 * are guaranteed accurate. (and year will not be less than 2 nor
 * greater than 9998).
 */
{
	int		lastmonth;
	int		lastyear;
	int		nextmonth;
	int		nextyear;

	lastyear = nextyear = thisyear;
	if ((lastmonth = thismonth - 1) == 0) {
	    lastmonth = 12;
	    lastyear--;
	}
	if ((nextmonth = thismonth + 1) == 13) {
	    nextmonth = 1;
	    nextyear++;
	}
	printf("%9s%5d%18s%5d%18s%5d\n",
		monthname[lastmonth], lastyear,
		monthname[thismonth], thisyear,
		monthname[nextmonth], nextyear);
	printf("%s   %s   %s\n", weekday, weekday, weekday);
	calendar(lastyear, lastmonth, 0);
	calendar(thisyear, thismonth, 1);
	calendar(nextyear, nextmonth, 2);
	output(3);
#if MONTHS_PER_LINE != 3
	<< error, the above won't work >>
#endif
	printf("\n\n\n");
}
	
output(nmonths)
int		nmonths;		/* Number of months to do	*/
/*
 * Clean up and output the text.
 */
{
	register int	week;
	register int	month;
	register char	*outp;

	for (week = 0; week < WEEKS_PER_MONTH; week++) {
	    outp = outline;
	    for (month = 0; month < nmonths; month++) {
		/*
		 * The -1 in the following removes
		 * the unwanted leading blank from
		 * the entry for Sunday.
		 */
		sprintf(outp, "%.*s%*s",
		    DAYS_PER_WEEK * ENTRY_SIZE - 1,
		    &layout[month][week][0][1],
		    MONTH_SPACE, "");
		outp += (DAYS_PER_WEEK * ENTRY_SIZE) + MONTH_SPACE - 1;
	    }
	    while (outp > outline && outp[-1] == ' ')
		outp--;
	    *outp = EOS;
	    puts(outline);
	}
}

calendar(year, month, index)
int		year;
int		month;
int		index;		/* Which of the three months		*/
/*
 * Actually build the calendar for this month.
 */
{
	register char	*tp;
	int		week;
	register int	wday;
	register int	today;

	setmonth(year, month);
	for (week = 0; week < WEEKS_PER_MONTH; week++) {
	    for (wday = 0; wday < DAYS_PER_WEEK; wday++) {
		tp = &layout[index][week][wday][0];
		*tp++ = ' ';
		today = getdate(week, wday);
		if (today <= 0) {
		    *tp++ = ' ';
		    *tp++ = ' ';
		}
		else if (today < 10) {
		    *tp++ = ' ';
		    *tp   = (today + '0');
		}
		else {
		    *tp++ = (today / 10) + '0';
		    *tp   = (today % 10) + '0';
		}
	    }
	}
}

usage(what, value)
char		*what;
int		value;
/*
 * Fatal parameter error
 */
{
	fprintf(stderr, "Calendar parameter error: bad %s: %d\n",
	    what, value);
	fprintf(stderr, "Usage: \"calend month\" or \"calend year month\"\n");
	fprintf(stderr, "Year and month are integers.\n");
	exit(IO_ERROR);
}

/*
 * Calendar routines, intended for eventual porting to TeX
 *
 * date(year, month, week, wday)
 *	Returns the date on this week (0 is first, 5 last possible)
 *	and day of the week (Sunday == 0)
 *	Note: January is month 1.
 *
 * setmonth(year, month)
 *	Parameters are as above, sets getdate() for this month.
 *
 * int
 * getdate(week, wday)
 *	Parameters are as above, uses the data set by setmonth()
 */

/*
 * This structure is used to pass data between setmonth() and getdate().
 * It needs considerable expansion if the Julian->Gregorian change is
 * to be extended to other countries.
 */

static struct {
    int		feb;		/* Days in February for this month	*/
    int		sept;		/* Days in September for this month	*/
    int		days_in_month;	/* Number of days in this month		*/
    int		dow_first;	/* Day of week of the 1st day in month	*/
} info;

static int day_month[] = {	/* 30 days hath September...		*/
	0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

int
date(year, month, week, wday)
int		year;		/* Calendar date being computed		*/
int		month;		/* January == 1				*/
int		week;		/* Week in the month 0..5 inclusive	*/
int		wday;		/* Weekday, Sunday == 0			*/
/*
 * Return the date of the month that fell on this week and weekday.
 * Return zero if it's out of range.
 */
{
	setmonth(year, month);
	return (getdate(week, wday));
}

setmonth(year, month)
int		year;			/* Year to compute		*/
int		month;			/* Month, January is month 1	*/
/*
 * Setup the parameters needed to compute this month
 * (stored in the info structure).
 */
{
	register int		i;

	if (month < 1 || month > 12) {	/* Verify caller's parameters	*/
	    info.days_in_month = 0;	/* Garbage flag			*/
	    return;
	}
	info.dow_first = Jan1(year);	/* Day of January 1st for now	*/
	info.feb = 29;			/* Assume leap year		*/
	info.sept = 30;			/* Assume normal year		*/
	/*
	 * Determine whether it's an ordinary year, a leap year
	 * or the magical calendar switch year of 1752.
	 */
	switch ((Jan1(year + 1) + 7 - info.dow_first) % 7) {
	case 1:				/* Not a leap year		*/
	    info.feb = 28;
	case 2:				/* Ordinary leap year		*/
	    break;

	default:			/* The magical moment arrives	*/
	    info.sept = 19;		/* 19 days hath September	*/
	    break;
	}
	info.days_in_month =
	      (month == 2) ? info.feb
	    : (month == 9) ? info.sept
	    : day_month[month];
	for (i = 1; i < month; i++) {
	    switch (i) {		/* Special months?		*/
	    case 2:			/* February			*/
		info.dow_first += info.feb;
		break;

	    case 9:
		info.dow_first += info.sept;
		break;

	    default:
		info.dow_first += day_month[i];
		break;
	    }
	}
	info.dow_first %= 7;		/* Now it's Sunday to Saturday	*/
}

int
getdate(week, wday)
int		week;
int		wday;
{
	register int	today;

	/*
	 * Get a first guess at today's date and make sure it's in range.
	 */
	today = (week * 7) + wday - info.dow_first + 1;
	if (today <= 0 || today > info.days_in_month)
	    return (0);
	else if (info.days_in_month == 19 && today >= 3) /* Magic time?	*/
	    return (today + 11);	/* If so, some dates changed	*/
	else				/* Otherwise,			*/
	    return (today);		/* Return the date		*/
}

static int
Jan1(year)
int		year;
/*
 * Return day of the week for Jan 1 of the specified year.
 */
{
	register int	day;

	day = year + 4 + ((year + 3) / 4);	/* Julian Calendar	*/
	if (year > 1800) {			/* If it's recent, do	*/
	    day -= ((year - 1701) / 100);	/* Clavian correction	*/
	    day += ((year - 1601) / 400);	/* Gregorian correction	*/
	}
	if (year > 1752)			/* Adjust for Gregorian	*/
	    day += 3;				/* calendar		*/
	return (day % 7);
}

