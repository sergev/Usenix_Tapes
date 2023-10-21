/*
 * Modification History 
 *
 * Origional Author:  Tom Stoehn@Tektronics[zeus!tims] Modifications by:  Marc
 * Ries@TRW[trwrb!ries] 
 *
 */

#include <curses.h>
#include "month.h"

/*
 * A routine that will print out the schedule for the day, i.e., 
 *
 * 8:45 - 9:45  Foo Bar Baz and so on... 
 */

extern short    nagflag, month, day, year;
extern char    *nagprog;

psched()
{
short           i, shour, sminute, ehour, eminute, n;
struct event_rec *events_today[MAX_EVENTS];

	get_daily_events(events_today);
	for (n = 0; events_today[n]; n++);
	sort_events(events_today, n);

	clear();
	move(0, 0);
	i = 0;
	while (events_today[i])
	{

		shour = events_today[i]->hour;
		sminute = events_today[i]->minute;
		ehour = shour + (events_today[i]->span_hours);
		eminute = sminute + (events_today[i]->span_minutes);

		printw("%2d:%02d%2s-%2d:%02d%2s %s\n", 
	               (shour <= 12) ? shour : (shour % 12), sminute,
		       ((shour < 12) ? "AM" : "PM"),
		       (ehour <= 12) ? ehour : (ehour % 12), eminute,
		       (((ehour % 24) < 12) ? "AM" : "PM"),
		       events_today[i]->event_string);
		i++;
	}
	refresh();
	get_char();
	clear();
	print_screen();
}

psched2()
{
short           i, shour, sminute, ehour, eminute, n;
struct event_rec *events_today[MAX_EVENTS];

	get_daily_events(events_today);
	for (n = 0; events_today[n]; n++);
	sort_events(events_today, n);

	i = 0;
	while (events_today[i])
	{

		shour = events_today[i]->hour;
		sminute = events_today[i]->minute;
		ehour = shour + (events_today[i]->span_hours);
		eminute = sminute + (events_today[i]->span_minutes);

		if (nagflag)
		{
			(void) printf(" %02d/%02d/%d", month, day, year);
			(void) printf(" %02d:%02d%2s ",
			       (shour <= 12) ? shour : (shour % 12), sminute,
			       ((shour < 12) ? "AM" : "PM"));
			(void) printf("-20:-9:-6:-4:-2 ");
			(void) printf("%s \"%s at \$then in \$pretime minutes.\"\n",
			       nagprog, events_today[i]->event_string);
		} else
		{
			(void) printf("%2d:%02d%2s-%2d:%02d%2s ",
			       (shour <= 12) ? shour : (shour % 12), sminute,
			       ((shour < 12) ? "AM" : "PM"),
			       (ehour <= 12) ? ehour : (ehour % 12), eminute,
			       ((ehour < 12) ? "AM" : "PM"));
			(void) printf("%s\n", events_today[i]->event_string);
		}
		i++;
	}
}
