/*
 * \usr\decusc\lib\time.c
 *
 * long time (long *result)
 *
 *	Returns and stores seconds since midnight, 1 January 1970
 *
 * Environment:	MS-DOS version 2.xx or later
 * Language:	Computer Innovations C86 version 2.10
 * Author:	Larry Campbell
 * Created:	June 1, 1984
 * Bugs:	Stops working Dec. 31, 2004
 */

static unsigned int
    days_in_year[] =		/* starting with 1980 */
	{ 366, 365, 365, 365, 366, 365, 365, 365, 366, 365, 365, 365,
	  366, 365, 365, 365, 366, 365, 365, 365, 366, 365, 365, 365},

    days_in_preceding_months[] =
	{ 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

long time (result)
long *result;
{
unsigned int
    y,
    year,
    month,
    day,
    hour,
    minute,
    second,
    Days;

unsigned long
    Time;

year = dos_year ();
month = dos_month ();
day = dos_date ();
hour = dos_hours ();
minute = dos_minutes ();
second = dos_seconds ();
Days = 3652;			/* days from 1-jan-70 to 1-jan-80 */
y = year - 1980;
while (y-- > 0)
    Days += days_in_year[y];
if (month > 1)
    Days += days_in_preceding_months[month - 2];
if (month > 2 && (year & 0x3))	/* account for February in leap years */
    Days++;
Days += day;
Time = second + (60L * (minute + 60L * hour));
Time = (86400L * Days) + Time;
if (result)
    *result = Time;
return (Time);
}
