/**		date.c		**/

/** return the current date and time in a readable format! **/
/** also returns an ARPA RFC-822 format date...            **/

/** (C) Copyright 1985, Dave Taylor **/

#include "headers.h"

#ifdef BSD
#  ifndef BSD4.1
#    include <sys/time.h>
#  else
#    include <time.h>
#    include <sys/types.h>
#    include <sys/timeb.h>
#  endif
#else
#  include <time.h>
#endif

#include <ctype.h>

#ifdef BSD
#undef toupper
#undef tolower
#endif

#define MONTHS_IN_YEAR	11	/* 0-11 equals 12 months! */
#define FEB		 1	/* 0 = January 		  */
#define DAYS_IN_LEAP_FEB 29	/* leap year only 	  */

#define ampm(n)		(n > 12? n - 12 : n)
#define am_or_pm(n)	(n > 11? (n > 23? "am" : "pm") : "am")
#define leapyear(year)	((year % 4 == 0) && (year % 100 != 0))

char *monname[] = { "January", "February", "March", "April", "May", "June",
		  "July", "August", "September", "October", "November",
		  "December", ""};

char *arpa_dayname[] = { "Sun", "Mon", "Tue", "Wed", "Thu",
		  "Fri", "Sat", "" };

char *arpa_monname[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", ""};

int  days_in_month[] = { 31,    28,    31,    30,    31,     30, 
		  31,     31,    30,   31,    30,     31,  -1};

#ifdef BSD
  char *timezone();
#else
  extern char *tzname[];
#endif

char *shift_lower(), *strcpy(), *strncpy();

char *get_arpa_date()
{
	/** returns an ARPA standard date.  The format for the date
	    according to DARPA document RFC-822 is exemplified by;

	       	      Mon, 12 Aug 85 6:29:08 MST

	**/

	static char buffer[SLEN];	/* static character buffer       */
	struct tm *the_time,		/* Time structure, see CTIME(3C) */
		  *localtime();
	long	   junk,		/* time in seconds....		 */
	    	   time();

#ifdef BSD
# ifdef BSD4.1
	struct timeb	loc_time;

	junk = time(long *) 0);
	ftime(&loc_time);
# else
	struct  timeval  time_val;		
	struct  timezone time_zone;

	gettimeofday(&time_val, &time_zone);
	junk = time_val.tv_sec;
# endif

#else
	junk = time((long *) 0);	/* this must be here for it to work! */
#endif

	the_time = localtime(&junk);

	sprintf(buffer, "%s, %d %s %d %d:%02d:%02d %s",
	  arpa_dayname[the_time->tm_wday],
	  the_time->tm_mday % 32,
	  arpa_monname[the_time->tm_mon],
	  the_time->tm_year % 100,
	  the_time->tm_hour % 24,
	  the_time->tm_min  % 61,
	  the_time->tm_sec  % 61,
#ifdef BSD
# ifdef BSD4.1
	  timezone(time_zone.timezone, the_time->tm_isdst));
# else
	  timezone(time_zone.tz_minuteswest, the_time->tm_isdst));
# endif
#else
	  tzname[the_time->tm_isdst]);
#endif
	
	return( (char *) buffer);
}

char *full_month(month)
char *month;
{
	/** Given a three letter month abbreviation, return the 
	    full name of the month.   If can't figure it out, just
	    return the given argument. **/

	char   name[4];
	char   *shift_lower();
	register int i;

	/** ensure name in correct case... **/

	strncpy(name, shift_lower(month), 3);
	name[0] = toupper(name[0]);

	/** now simply step through arpa_monname table to find a match **/

	for (i=0; i < 12; i++)
	  if (strncmp(name, arpa_monname[i], 3) == 0)
	    return((char *) monname[i]);
	
	dprint1(2, "Warning: Couldn't expand monthname %s (full_month)\n", 
	        month);

	return( (char *) month);
}

days_ahead(days, buffer)
int days;
char *buffer;
{
	/** return in buffer the date (Day, Mon Day, Year) of the date
	    'days' days after today. **/

	struct tm *the_time,		/* Time structure, see CTIME(3C) */
		  *localtime();
	long	   junk,		/* time in seconds....		 */
		   time();

	junk = time((long *) 0);	/* this must be here for it to work! */
	the_time = localtime(&junk);

	/* increment the day of the week */

	the_time->tm_wday = (the_time->tm_wday + days) % 7;

	/* the day of the month... */
	the_time->tm_mday += days;
	
	if (the_time->tm_mday > days_in_month[the_time->tm_mon]) {
	  if (the_time->tm_mon == FEB && leapyear(the_time->tm_year)) {
	    if (the_time->tm_mday > DAYS_IN_LEAP_FEB) {
	      the_time->tm_mday -= days_in_month[the_time->tm_mon];
	      the_time->tm_mon += 1;
	    }
	  }
	  else {
	    the_time->tm_mday -= days_in_month[the_time->tm_mon];
	    the_time->tm_mon += 1;
	  }
	}

	/* check the month of the year */
	if (the_time->tm_mon > MONTHS_IN_YEAR) {
	  the_time->tm_mon -= MONTHS_IN_YEAR;
	  the_time->tm_year += 1;
	}

	/* now, finally, build the actual date string */

	sprintf(buffer, "%s, %d %s %d",
	  arpa_dayname[the_time->tm_wday],
	  the_time->tm_mday % 32,
	  arpa_monname[the_time->tm_mon],
	  the_time->tm_year % 100);
}

int
valid_date(day, mon, year)
char *day, *mon, *year;
{
	/** Validate the given date - returns TRUE iff the date
	    handed is reasonable and valid.  
	    Ignore month param, okay? 
	**/

	register int daynum, yearnum;

	daynum = atoi(day);
	yearnum = atoi(year);
	
	if (daynum < 1 || daynum > 31) {
	  dprint1(3, "Error: day %d is obviously wrong! (valid_date)\n", 
	          daynum);
	  return(0);
	}
	
	if (yearnum < 1 || (yearnum > 100 && yearnum < 1900) ||
	    yearnum > 2000) {
	  dprint1(3, "Error: year %d is obviously wrong! (valid_date)\n", 
		yearnum);
	  return(0);
	}
	
	return(1);
}

fix_date(entry)
struct header_rec *entry;
{
	/** This routine will 'fix' the date entry for the specified
	    message.  This consists of 1) adjusting the year to 0-99
	    and 2) altering time from HH:MM:SS to HH:MM am|pm **/ 

	if (atoi(entry->year) > 99) 	
	  sprintf(entry->year,"%d", atoi(entry->year) - 1900);

	fix_time(entry->time);
}

fix_time(timestring)
char *timestring;
{
	/** Timestring in format HH:MM:SS (24 hour time).  This routine
	    will fix it to display as: HH:MM [am|pm] **/

	int hour, minute;

	sscanf(timestring, "%d:%d", &hour, &minute);

	if (hour < 1 || hour == 24) 
	  sprintf(timestring, "12:%2.2d (midnight)", minute);
	else if (hour < 12)
	  sprintf(timestring, "%d:%2.2d am", hour, minute);
	else if (hour == 12)
	  sprintf(timestring, "%d:%2.2d (noon)", hour, minute);
	else if (hour < 24)
	  sprintf(timestring, "%d:%2.2d pm", hour-12, minute);
}

int
compare_dates(rec1, rec2)
struct header_rec *rec1, *rec2;
{
	/** This function works similarly to the "strcmp" function, but
	    has lots of knowledge about the internal date format...
	    Apologies to those who "know a better way"...
	**/

	int month1, day1, year1, hour1, minute1,
	    month2, day2, year2, hour2, minute2;

	year1 = atoi(rec1->year);
	year2 = atoi(rec2->year);

	if (year1 != year2)
	  return( year1 - year2 );

	/* And HERE's where the performance of this sort dies... */

	month1 = month_number(rec1->month);	/* retch...  gag....  */
	month2 = month_number(rec2->month);	/*    puke...         */

	if (month1 == -1) 
	  dprint1(2,"month_number failed on month '%s'\n", rec1->month);

	if (month2 == -1) 
	  dprint1(2,"month_number failed on month '%s'\n", rec2->month);

	if (month1 != month2)
	  return( month1 - month2 );

	/* back and cruisin' now, though... */

	day1 = atoi(rec1->day);	 /* unfortunately, 2 is greater than 19  */
	day2 = atoi(rec2->day);  /* on a dump string-only compare...     */

	if (day1 != day2)
	  return( day1 - day2 );

	/* we're really slowing down now... */

	minute1 = minute2 = -1;

	sscanf(rec1->time, "%d:%d", &hour1, &minute1);
	sscanf(rec2->time, "%d:%d", &hour2, &minute2);

	/* did we get the time?  If not, try again */

	if (minute1 < 0)
	  sscanf(rec1->time, "%2d%2d", &hour1, &minute1);

	if (minute2 < 0)
	  sscanf(rec2->time, "%2d%2d", &hour2, &minute2);

	/** deal with am/pm, if present... **/

	if (strlen(rec1->time) > 3)
	  if (rec1->time[strlen(rec1->time)-2] == 'p')
	    hour1 += 12;

	if (strlen(rec2->time) > 3)
	  if (rec2->time[strlen(rec2->time)-2] == 'p')
	    hour2 += 12;

	if (hour1 != hour2)
	  return( hour1 - hour2 );

	return( minute1 - minute2 );		/* ignore seconds... */
}

int
compare_parsed_dates(rec1, rec2)
struct date_rec rec1, rec2;
{
	/** This function is very similar to the compare_dates
	    function but assumes that the two record structures
	    are already parsed and stored in "date_rec" format.
	**/

	if (rec1.year != rec2.year)
	  return( rec1.year - rec2.year );
	
	if (rec1.month != rec2.month)
	  return( rec1.month - rec2.month );

	if (rec1.day != rec2.day)
	  return( rec1.day - rec2.day );

	if (rec1.hour != rec2.hour)
	  return( rec1.hour - rec2.hour );

	return( rec1.minute - rec2.minute );		/* ignore seconds... */
}

int
month_number(name)
char *name;
{
	/** return the month number given the month name... **/

	char ch;

	switch (tolower(name[0])) {
	 case 'a' : if ((ch = tolower(name[1])) == 'p')	return(APRIL);
		    else if (ch == 'u') return(AUGUST);
		    else return(-1);	/* error! */
	
	 case 'd' : return(DECEMBER);
	 case 'f' : return(FEBRUARY);
	 case 'j' : if ((ch = tolower(name[1])) == 'a') return(JANUARY);
		    else if (ch == 'u') {
	              if ((ch = tolower(name[2])) == 'n') return(JUNE);
		      else if (ch == 'l') return(JULY);
		      else return(-1);		/* error! */
	            }
		    else return(-1);		/* error */
	 case 'm' : if ((ch = tolower(name[2])) == 'r') return(MARCH);
		    else if (ch == 'y') return(MAY);
		    else return(-1);		/* error! */
	 case 'n' : return(NOVEMBER);
	 case 'o' : return(OCTOBER);
	 case 's' : return(SEPTEMBER);
	 default  : return(-1);
	}
}

#ifdef SITE_HIDING

char *get_ctime_date()
{
	/** returns a ctime() format date, but a few minutes in the 
	    past...(more cunningness to implement hidden sites) **/

	static char buffer[SLEN];	/* static character buffer       */
	struct tm *the_time,		/* Time structure, see CTIME(3C) */
		  *localtime();
	long	   junk,		/* time in seconds....		 */
		   time();
#ifdef BSD
	struct  timeval  time_val;		
	struct  timezone time_zone;
#endif

#ifdef BSD
	gettimeofday(&time_val, &time_zone);
	junk = time_val.tv_sec;
#else
	junk = time((long *) 0);	/* this must be here for it to work! */
#endif
	the_time = localtime(&junk);

	sprintf(buffer, "%s %s %d %02d:%02d:%02d %d",
	  arpa_dayname[the_time->tm_wday],
	  arpa_monname[the_time->tm_mon],
	  the_time->tm_mday % 32,
	  min(the_time->tm_hour % 24, (rand() % 24)),
	  min(abs(the_time->tm_min  % 61 - (rand() % 60)), (rand() % 60)),
	  min(abs(the_time->tm_sec  % 61 - (rand() % 60)), (rand() % 60)),
	  the_time->tm_year % 100 + 1900);
	
	return( (char *) buffer);
}

#endif
