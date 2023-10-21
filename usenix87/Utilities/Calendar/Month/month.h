#define VERSION		"1.4 (USENET) 03/01/87" /* Version number!  SCCS should agree */

#define TOP_MONTH_ROW 3
#define YEAR_ROW 16
#define YEAR_COL 12
#define TIME_ROW 20
#define DURATION_ROW 21
#define DESCRIPTION_ROW 22
#define ACCEPT_ROW 23
#define LAST_YEAR_COL 75
#define DATE_COL 0
#define MONTHLY_COL 15
#define YEARLY_COL 23
#define EVERY_COL 30
#define NTH_COL 36
#define LAST_COL 41
#define SMTWTFS_COL 46
#define TIME_COL 11
#define MINUTE_COL 14
#define ACCEPT_COL 11
#define CANCEL_COL 18
#define SMONTH_COL 0
#define SDAY_COL 3
#define SYEAR_COL 6

#define MONTHS 0
#define YEARS 1
#define DAYS 2
#define SCHEDULE 3

#define MAX_EVENTS 250

#define MAX_EVENT_STRING_LENGTH 70

#define NOTHING 0
#define ACCEPT 1
#define CANCEL 2

struct event_rec {
	char event_month;		/* month of the event */
	char event_day;			/* day of the event */
	short event_year;		/* year of the event */
	char monthly;			/* does event occur monthly? */
	char yearly;			/* does event occur yearly? */
	char every;				/* does event occur on every something? */
	char smtwtfs[7];		/* which days of the week are relevent? */
	char nth, last;			/* does event occurn on an nth or last somehting? */
	char nth_is_on;			/* is 'nth' selected by user, n is nth above */
	char hour;				/* hour of the event */
	char minute;			/* minute of the event */
	char span_hours;	/* hours event lasts */
	char span_minutes;	/* minutes event lasts, multiple of 15 */
	char event_string[MAX_EVENT_STRING_LENGTH];	/* short description of event */
	struct event_rec *next_event;	/* next event */
};

struct mdate {
	short month;
	short year;
};
