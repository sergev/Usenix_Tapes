#include <stdio.h>
#include <pwd.h>
#include <sgtty.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#ifdef DEBUG
#	define DEB(x)	fprintf/**/x
#else
#	define DEB(x)
#endif DEBUG

#define when		break; case
#define otherwise	break; default
#define nitems(x)	(sizeof(x) / sizeof(x[0]))

#define IOCTL(op, par)			\
	if (ioctl(0, op, par) < 0)	\
	{				\
		perror("tty");		\
		exit(-1);		\
	}

#define NEW(t)	(t *) malloc(sizeof(t))
#define FREENODE			\
	{				\
		TIMES *tmp = alarms;	\
					\
		alarms = alarms->next;	\
		free(tmp);		\
	}

#define CONFIG_FILE	".alarm"
#define RESET_FILE	".alarmreset"
#define RESET		"kill -%d %ld"
#define SIGREAD		SIGEMT

#define QUERY		"\033[11t"
#define RESPONSE	"\033[%dt"
#define ICONIC		2

#define SIGNAL(x)	printf(alarm_/**/x)
#define ICON_DIR	"/unc/black/icons"
#define IMAGE		"\033]I%s/alarm_%s\033\134"
#define IHEADER		"\033]L\033\134"

/*
**  Time constants
*/
#define SPM	 60			/* seconds per minute */
#define SPH	(60 * SPM)		/* seconds per hour */
#define SPD	(24 * SPH)		/* seconds per day */
#define LOOK	(SPD >> 1)
#define JAN_86	504921600L		/* 1 January, 1986 */

/*
**  Types
*/
#define STRLEN		80
typedef char STRING[STRLEN];

typedef struct TIMES		/* the TIMES structure */
{
	int time;		/* how many seconds into the epoch */
	STRING message;
	struct TIMES *next;
} TIMES;

static TIMES *read_alarm_file();
static TIMES *insert();
static long gettime();
static int iconic();
static void usage();
static void setup_term();
static void lowercase();
static long date_to_time();
int reset_term();
int reset_alarm();
long time();

jmp_buf env;
char *pname;

/*
**  This program is a SUN alarmer.  It reads an alarm database,
**  then flashes the alarm bell whenever the appropriate time
**  occurs.
*/

main(argc, argv)
int argc;
char *argv[];
{
	STRING	alarm_on,	/* alarm on icon (normal) */
		alarm_inv,	/* alarm on icon (inverted) */
		alarm_off;	/* alarm off icon */
	STRING alarmdb;		/* alarm file name */
	STRING reset;		/* reset file name */
	int rfile;
	TIMES *alarms;		/* alarm structure */
	long getpid();
	char *getenv();
	struct passwd *getpwuid();
	struct passwd *p_entry;
	int history = -SPH;
	int i;
	char *icon_dir;

	pname = *argv;

	if ( (p_entry = getpwuid(getuid())) )
	{
		sprintf(alarmdb, "%s/%s", p_entry->pw_dir, CONFIG_FILE);
		sprintf(reset, "%s/%s", p_entry->pw_dir, RESET_FILE);
	}
	else
	{
		perror("getuid");
		exit(-1);
	}

	/*
	**  Take the arguments.  The only legal one now is an
	**  optional database file.  If one is not given, use
	**  the default.
	*/
	while (--argc)
		if (**++argv == '-')
			switch ((*argv)[1])
			{
				when 'h':  i = atoi(*++argv);
					   if ((i >= 0) && (i <= SPM))
						history = -60 * i;
					   --argc;
				otherwise: usage(pname);
			}
		else
			strcpy(alarmdb, *argv);


	/*
	**  Enable the user to signal this process
	*/
	if ((rfile = open(reset, O_TRUNC | O_WRONLY | O_CREAT, 0700)) == -1)
	{
		perror("open");
		exit(-1);
	}
	else
	{
		sprintf(reset, RESET, SIGREAD, getpid());
		write(rfile, reset, strlen(reset));
		close(rfile);
	}

	/*
	**  Set up the icons
	*/
	if (!(icon_dir = getenv("ICONDIR")))
		icon_dir = ICON_DIR;
	sprintf(alarm_on,  IMAGE, icon_dir, "on");
	sprintf(alarm_inv, IMAGE, icon_dir, "on2");
	sprintf(alarm_off, IMAGE, icon_dir, "off");

	/*
	**  Read in the alarm file.
	*/
	DEB((stderr, "About to read alarm file\n"));
	alarms = read_alarm_file(alarmdb, history);

	/*
	**  Set up the icon and the tty driver.
	*/
	setup_term();
	printf(IHEADER);
	SIGNAL(off);
	fflush(stdout);

	/*
	**  Set up signal trapping.
	*/
	for (i = SIGHUP; i <= SIGPROF; ++i)
		signal(i, reset_term);

	signal(SIGREAD, reset_alarm);

	/*
	**  Trap SIGREAD.
	*/
	if (setjmp(env))
	{
		while (alarms)
			FREENODE;
		alarms = read_alarm_file(alarmdb, 0);
		signal(SIGREAD, reset_alarm);
	}

	/*
	**  Repeat ad nauseum.
	*/
	for (;;)
	{
		/*
		**  Sleep until the next alarm.
		*/
		if (alarms->time > time(0))
		{
			DEB((stderr, "Sleeping for %ld seconds.\n",
				alarms->time - time(0)));
			sleep((int) (alarms->time - time(0)));
		}
		else
			DEB((stderr, "Not sleeping.\n"));

		/*
		**  Check to see if we need to re-read the file.
		*/
		if (!alarms->next)
		{
			FREENODE;
			alarms = read_alarm_file(alarmdb, 0);
		}
		else
		{
			/*
			**  Indicate the alarm, and wait for the user
			**  to open the window.
			*/
			while (iconic())
			{
				SIGNAL(on);
				SIGNAL(inv);
			}

			/*
			**  Dump ALL current alarm messages.
			*/
			putchar('\n');
			do
			{
				puts(alarms->message);
				FREENODE;
			} while (alarms->next && (alarms->time <= time(0)));

			/*
			**  Turn the clock off.
			*/
			SIGNAL(off);
			fflush(stdout);
		}
	}
}

struct sgttyb save_tty;

static void
setup_term()
{
	struct sgttyb tty;

	IOCTL(TIOCGETP, &tty);
	save_tty = tty;

	tty.sg_flags &= ~ECHO;
	tty.sg_flags |= CBREAK;

	IOCTL(TIOCSETP, &tty);
}

int
reset_term(sig)
int sig;
{
	IOCTL(TIOCSETP, &save_tty);
	exit(sig);
}

int
reset_alarm()
{
	longjmp(env);
}

int daily;
int cwday,	/* day of week */
    cday,	/* day of month */
    cmonth,	/* month of year */
    cyear;	/* year */
int TZ;		/* difference to GMT */

static TIMES *
read_alarm_file(fname, history)
STRING fname;
int history;
{
	FILE *fopen();
	struct tm *localtime();
	FILE *alarmdb;
	TIMES *alarms, *cur;
	char input[160];
	long ctime;		/* current time-of-day */
	long itime;		/* alarm entry time-of-day */
	long rtime;		/* relative to current time-of-day */
	struct tm *ltime;
	struct timeb tp;

	/*
	**  Open the alarm database
	*/
	if (!(alarmdb = fopen(fname, "r")))
	{
		perror(fname);
		usage(pname);
	}

	/*
	**  Calculate the current time of day.
	*/
	ftime(&tp);
	ctime = tp.time;
	TZ = tp.timezone * SPM;
	ltime = localtime(&ctime);
	DEB((stderr, "Current time = %ld\n", ctime));
	cmonth = ltime->tm_mon + 1;
	cday = ltime->tm_mday;
	cyear = ltime->tm_year;
	cwday = ltime->tm_wday - 7;
	DEB((stderr, "cmonth = %d, cday = %d, cyear = %d, cwday = %d\n",
		cmonth, cday, cyear, cwday));

	/*
	**  Read the alarms file and put each alarm
	**  in the structure, sorted by time.  Only
	**  alarms in the next 12 hours are entered.
	*/
	alarms = NEW(TIMES);
	alarms->time = ctime + LOOK;
	strcpy(alarms->message, "12 hour timer interrupt");
	alarms->next = NULL;

	while (fgets(input, sizeof(input), alarmdb))
	{
		input[strlen(input) - 1] = '\0';
		rtime = (itime = gettime(input)) - ctime;

		if (daily)
		{
			if (rtime > LOOK)
			{
				rtime -= SPD;
				itime -= SPD;
			}
			else if (rtime < -LOOK)
			{
				rtime += SPD;
				itime += SPD;
			}
		}

		if ((rtime < LOOK) && (rtime >= history))
		{
			cur = NEW(TIMES);
			cur->time = itime;
			strcpy(cur->message, input);

			alarms = insert(alarms, cur);
		}
	}

	fclose(alarmdb);
	traverse(alarms);
	return(alarms);
}

#define ISDAY(s) !strncmp(dow, s, strlen(dow))

static long
gettime(input)
char *input;
{
	int date[5];
	STRING dow;	/* for weekly or daily alarms */
	static STRING days[] =
	{
		"sundays",
		"mondays",
		"tuesdays",
		"wednesdays",
		"thursdays",
		"fridays",
		"saturdays"
	};

	daily = 0;
	if ((sscanf(input, "%s", dow) != 1) || (*dow == '#'))
		return(0L);
	if (isdigit(*dow))
		sscanf(input, "%d/%d %d:%d %[^\n]",
			&date[0], &date[1], &date[3], &date[4]);
	else
	{
		register int i;

		sscanf(input, "%s %d:%d %[^\n]", dow, &date[3], &date[4]);
		lowercase(dow);
		*date = cmonth;

		if (ISDAY("any") || ISDAY("daily"))
		{
			date[1] = cday;
			daily = 1;
		}
		else
		{
			for (i = 0; i < nitems(days); ++i)
				if (ISDAY(days[i]))
				{
					date[1] = cday + ((i - cwday) % 7);
					break;
				}
			if (i == nitems(days))
			{
				fprintf(stderr,
					"'%s' is an invalid alarm entry.\n",
					input);
				return(0L);
			}
		}

		DEB((stderr, "periodic reminder: %s becomes %d/%d\n",
				dow, date[0], date[1]));
	}

	if ((date[0] > cmonth) || ((date[0] == cmonth) && (date[1] >= cday)))
		date[2] = cyear;
	else
		date[2] = cyear + 1;

	return(date_to_time(date));
}

static void
lowercase(str)
char *str;
{
	for ( ; *str; ++str)
		if (isupper(*str))
			*str = tolower(*str);
}

/*
**  Take an array of [month day year hour minute]
**  and return the number of seconds since the epoch.
*/
static long
date_to_time(date)
int *date;
{
	static int corr[] =
	{
		0, 1, -1, 0, 0, 1, 1, 2, 3, 3, 4, 4
	};
	int month  = *date - 1;
	int day  = *++date - 1;
	int year = *++date;
	int hour = *++date;
	int min = *++date;
	int leap = ((month > 1) && (dysize(year) == 366));
	register int ydisp = (30 * month + day + corr[month]) * SPD;
	register int i;

	if (leap)
		ydisp += SPD;

	for (i = 86; i < year; ++i)
		ydisp += (SPD + dysize(i));

	DEB((stderr, "Time for %d/%d, %d at %d:%2d = %ld\n",
			month+1, day+1, year, hour, min,
			JAN_86 + TZ + ydisp + (hour * SPH) + (min * SPM)));

	return(JAN_86 + TZ + ydisp + (hour * SPH) + (min * SPM));
}

/*
**  Perform an insertion sort.
*/
static TIMES *
insert(node, el)
TIMES *node, *el;
{
	TIMES *last = NULL;
	TIMES *root = node;

	DEB((stderr, "Inserting node.  time = %ld, msg = '%s'\n",
			el->time, el->message));
	while (node->next && (node->time < el->time))
	{
		last = node;
		node = node->next;
	}

	el->next = node;
	if (last)
		last->next = el;
	else
		root = el;

	return(root);
}

traverse(alarms)
TIMES *alarms;
{
#ifdef DEBUG
	putc('\n', stderr);
	for ( ; alarms; alarms = alarms->next)
		fprintf(stderr, "alarm at time = %ld; msg = %s\n",
			alarms->time, alarms->message);
#endif DEBUG
}

static int
iconic()
{
	int ans;

	IOCTL(TIOCFLUSH, 0);
	printf(QUERY);
	scanf(RESPONSE, &ans);

	return(ans == ICONIC);
}

static void
usage(pname)
char *pname;
{
	printf("usage: %s [-h history] [alarmfile]\n", pname);
	exit(-1);
}
