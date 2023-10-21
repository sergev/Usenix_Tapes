/**		mydate.c		**/

/** This program outputs the date in the format:
	DAY, MONTH DATE YEAR at HH:MM am|pm
	("Monday, January 17th, 1985 at 4:17 pm" for example)
    or, with ANY parameters at all;
	DAY, MONTH DATE YEAR
        without any 'time' string...

    Dave Taylor, Colorado Networks Operation
**/

#include <time.h>
#include <stdio.h>

#define ampm(n)		(n > 12? n - 12 : n)
#define am_or_pm(n)	(n > 12? "pm" : "am")

char *dayname[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
		  "Friday", "Saturday", "" };

char *monname[] = { "January", "February", "March", "April", "May", "June",
		  "July", "August", "September", "October", "November",
		  "December", ""};

main(argc, argv)
int argc;
char *argv[];
{
	struct tm *the_time;		/* Time structure, see CTIME(3C) */
	char      *suffix();	 	/* digit suffix for date	 */
	long	  junk;			/* time in seconds....		 */

	junk = time(0);	/* this must be here for it to work! */
	the_time = (struct tm * ) localtime(&junk);
		
	if (argc == 1)  	/* normal format... */
	  printf("%s, %s %d%s %d at %d:%02d %s\n",
		 dayname[the_time->tm_wday],	/* weekday */
		 monname[the_time->tm_mon],	/* month   */
		 the_time->tm_mday,		/* day     */
		 suffix(the_time->tm_mday),	/* suffix  */
		 the_time->tm_year + 1900,	/* year    */
		 ampm(the_time->tm_hour),	/* hour    */
		 the_time->tm_min,		/* minute  */
		 am_or_pm(the_time->tm_hour));	/* am | pm */
	else 			/* sccs (retch) format */
	  printf("%s, %s %d%s %d\n",
		 dayname[the_time->tm_wday],	/* weekday */
		 monname[the_time->tm_mon],	/* month   */
		 the_time->tm_mday,		/* day     */
		 suffix(the_time->tm_mday),	/* suffix  */
		 the_time->tm_year + 1900);	/* year    */
}

char *suffix(day)
int day;
{
	/** this routine returns the suffix appropriate for the
	    specified number to make it an ordinal number.  ie,
	    if given '1' it would return 'st', and '2' => 'nd'
	**/

	static char buffer[10];
	register int digit;

	digit = day % 10;

	if (digit == 0 || digit > 3 || (day > 10 && day < 14))
	  strcpy(buffer,"th");
	else if (digit == 1)
	  strcpy(buffer,"st");
	else if (digit == 2)
	  strcpy(buffer, "nd");
	else
	  strcpy(buffer, "rd");

	return( (char *) buffer);
}
