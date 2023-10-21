/*
 * Modification History 
 *
 * Origional Author:  Tom Stoehn@Tektronics[zeus!tims] Modifications by:  Marc
 * Ries@TRW[trwrb!ries] 
 *
 */

#ifdef BSD
#  include <sys/time.h>
#else
#include <sys/types.h>
#  include <time.h>
#endif

#include <ctype.h>

#ifdef BSD
# undef tolower
# undef toupper
#endif

#include "month.h"

#ifdef BSD
char           *timezone();
#else
extern char    *tzname[];
#endif


short           month, year, day;
short           this_month, this_day, this_year;
short           start_day;
long            time();

extern short    dhour, dminute, dsecond, days;

get_current_date()
{
struct tm      *tmp,	/* Time structure, see CTIME(3C) */
               *localtime();
long            junk;	/* time in seconds....		 */
#ifdef BSD
struct timeval  tp;
struct timezone tzp;
#endif

#ifdef BSD
	(void) gettimeofday(&tp, &tzp);
	junk = tp.tv_sec;
#else
	junk = (long) time(0);	/* this must be here for it to work! */
#endif
	tmp = localtime(&junk);
	year = this_year = 1900 + tmp->tm_year;
	month = this_month = tmp->tm_mon + 1;
	day = this_day = tmp->tm_mday;
	dhour = tmp->tm_hour;
	dminute = tmp->tm_min;
	dsecond = tmp->tm_sec;

	start_day = get_start_day(this_month, this_year);
}

jan1(year)
	register        year;
{
register        day;

	day = 4 + year + (year + 3) / 4;

	if (year > 1800)
	{
		day -= (year - 1701) / 100;
		day += (year - 1601) / 400;
	}
	if (year > 1752)
		day += 3;

	return (day % 7);
}

is_leap_year(year)
	int             year;
{
int             day;

	day = jan1(year);
	return ((((jan1(year + 1) + 7 - day) % 7) == 2) ? 1 : 0);
}

get_start_day(month, year)
	register        month, year;
{
short           day, i;

	day = jan1(year);

	for (i = 1; i < month; i++)
	{
		day = (day + days_in(i, year)) % 7;
	}
	return (day);
}

days_in(month, year)
	register        month, year;
{
int             days;

	switch (month)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		days = 31;
		break;
	case 4:
	case 6:
	case 9:
	case 11:
		days = 30;
		break;
	case 2:
		days = 28 + is_leap_year(year);
		break;
	}
	return (days);
}

is_passed_event(event)
	struct event_rec *event;
{
	if (event->monthly || event->yearly || event->every ||
	    (event->event_year > this_year))
	{
		return (0);
	}
	if (event->event_year < this_year)
	{
		return (1);
	}
	/* now we know it's this year */
	if (event->event_month > this_month)
	{
		return (0);
	}
	if (event->event_month < this_month)
	{
		return (1);
	}
	/* now we know it's this month */
	if (event->event_day < this_day)
	{
		return (1);
	}
	return (0);
}

is_before(m, d, y, month, day, year)
	register        m, d, y, month, day, year;
{
	if (y < year)
	{
		return (1);
	}
	if (y > year)
	{
		return (0);
	}
	if (m < month)
	{
		return (1);
	}
	if (m > month)
	{
		return (0);
	}
	if (d < day)
	{
		return (1);
	}
	return (0);
}

has_smtwtfs(smtwtfs)
	register char  *smtwtfs;
{
register        i;

	for (i = 0; i < 7; i++)
	{
		if (smtwtfs[i])
		{
			return (1);
		}
	}
	return (0);
}

event_matches_date(event)
	register struct event_rec *event;
{
short           last;
int             n;
	/* check if current date is before start date of event */

	if (is_before(month, day, year, event->event_month, event->event_day,
		      event->event_year))
	{
		return (0);
	}
	/* one time events */

	if ((event->event_year == year) && (event->event_month == month) &&
	    (event->event_day == day) && !event->every &&
	    !event->nth_is_on && !event->last)
	{
		return (1);
	}
	/* once monthly or once yearly events */

	if (!event->every && !event->nth_is_on && !event->last)
	{
		if (event->monthly)
		{
			if (event->event_day == day)
			{
				return (1);
			}
		} else if (event->yearly)
		{
			if ((event->event_month == month) &&
			    (event->event_day == day))
			{
				return (1);
			}
		}
	}
	if ((event->monthly || event->yearly) && !event->every &&
	    !event->nth_is_on && !event->last)
	{
		if (event->monthly && (event->event_day == day))
		{
			return (1);
		}
		if (event->yearly && (event->event_month == month) &&
		    (event->event_day == day))
		{
			return (1);
		}
	}
	if (!event->smtwtfs[(day - 1 + start_day) % 7])
	{
		return (0);
	}
	/* everys */

	if (event->every)
	{

		/* every smtwtf */

		if (!event->nth_is_on && !event->last)
		{
	EVDAY:		if (event->smtwtfs[((day - 1 + start_day) % 7)])
			{
				return (1);
			}
			return (0);
		}
		/* every monthly/yearly */

		if (event->monthly || event->yearly)
		{
			/* every monthly not-1st2nd3rdlast */
			if (!event->nth_is_on && !event->last)
			{
				goto EVDAY;
			}
			/* every monthly/yearly with one of 1st2nd3rdlast */

			if (event->nth_is_on)
			{
				if (event->monthly &&
				    (nth_smtwtfs_of_month(&last) ==
				     (event->nth - 1)))
				{
					return (1);
				}
				if (event->yearly &&
				    (nth_smtwtfs_of_year(&last) ==
				     (event->nth - 1)))
				{
					return (1);
				}
			}
			if (event->last)
			{
				if (event->monthly)
				{
					(void) nth_smtwtfs_of_month(&last);
					if (last)
					{
						return (1);
					}
				}
				if (event->yearly)
				{
					(void) nth_smtwtfs_of_year(&last);
					if (last)
					{
						return (1);
					}
				}
			}
		} else
		{
			/* every not-monthly and not-yearly */
			if (!event->nth_is_on && !event->last)
			{
				goto EVDAY;
			}
			/* every nth/dayofweek */
			if (event->nth == 1)
			{
				return (1);
			}
			n = how_many_since(event->event_month,
					event->event_day, event->event_year);
			if ((n % (event->nth)) == 1)
			{
				return (1);
			}
		}
	}
	return (0);
}

nth_smtwtfs_of_month(last)
	short          *last;
{
	*last = ((day + 7) > days) ? 1 : 0;
	return ((day - 1) / 7);
}

nth_smtwtfs_of_year(last)
	short          *last;
{
short           days, i;

	for (i = 1, days = day; i < month; i++)
	{
		days += days_in(i, year);
	}

	*last = ((days + 7) > (365 + is_leap_year(year))) ? 1 : 0;
	return ((days - 1) / 7);
}

how_many_since(m, d, y)
	int             m, d, y;
{
register        total_days_passed;
int             i;

	if (y < year)
	{
		total_days_passed = 0;
		for (i = m; i <= 12; i++)
		{
			total_days_passed += days_in(i, y);
		}
		total_days_passed -= (d - 1);
		for (i = (y + 1); i < year; i++)
		{
			total_days_passed += (365 + is_leap_year(i));
		}
		for (i = 1; i < month; i++)
		{
			total_days_passed += days_in(i, year);
		}
		total_days_passed += day;
	} else if (m == month)
	{
		total_days_passed = day - d + 1;
	} else
	{
		total_days_passed = 1 - d;
		for (i = m; i < month; i++)
		{
			total_days_passed += days_in(i, year);
		}
		total_days_passed += day;
	}
	return ((((total_days_passed - 1) / 7) + 1));
}

days_since_jan1(month, day, year)
{
int             days = 0, i;

	for (i = 1; i < month; i++)
	{
		days += days_in(i, year);
	}
	days += day;
	return (days);
}
