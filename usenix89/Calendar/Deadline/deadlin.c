/* This is a program to modify the prompt on a 4.2BSD system to print
a deadline time of number of days. Use is for important dates, such
as contract termination, weddings, ... etc (!).
In your .login put the line:
		set prompt = "`deadline`"
Assuming the compiled source is in your home directory, the prompt
will start at something  like
			1:1 (365) 
where 365 is the number of days to the deadline.
The code below was hacked out of a Biorhythms program which I wrote to
learn the language C: therefore it is far from optimal.
The method used to specify a deadline is (gulp) to edit the program:
I agree that this isnt elegant, but I leave it to some other hacker to
improve it.
The prog has been used for the past ~200 days successfully.
Bugs, ideas etc to myself please.
Have fun
Mungo Henning
*/

#include <stdio.h>
#include <sys/time.h>

days_in_year(year)
{
	if ( (year%400==0) || (year%100!=0 & year%4==0) ) return(366);
	else return(365);
}

/* This function calculates the days in a given month, in a given year */

days_in_month(month,year)
{
	switch (month)
	      {
		case 2:if (days_in_year(year) == 365) return(28);
			 else return(29);
		case 4:
		case 6:
		case 9:
		case 11: return(30);
		default: return(31);
	      }
}


/* declare structure for date */

struct date  {int _day, _month, _year};

between (before,after)
struct date before, after;
{
	int months, years,
	interval = days_in_month(before._month,before._year) - before._day + 1;

	for (months=before._month+1; months<13; months=months+1)
		interval = interval + days_in_month(months,before._year);

	for (years=before._year+1; years<=after._year; years=years+1)
		interval = interval + days_in_year(years);

	/* now subtract days left in 'after' year */

	for (months=after._month+1; months<13; months=months+1)
		interval = interval - days_in_month(months,after._year);

	/* finally subtract days left in 'after' month */

	interval = interval - ( days_in_month(after._month,after._year)-
				after._day + 1);
	return(interval);
}

main(argc,argv)
int argc; char **argv;
{
		int tim = time(0);
		char command[80];

		struct date start,finish;
		struct tm *tvec;

		tvec = localtime(&tim);
		start._year = tvec->tm_year+1900;
		start._month = tvec->tm_mon+1;
		start._day = tvec->tm_mday;

		finish._year = 1986;
		finish._month = 9;
		finish._day = 30;

		printf("1:! (%d) ",between(start,finish));
}
