/* #define LATTICE */
#ifndef LATTICE
#include <sys/time.h>			/* structures for time system calls */
#endif
#include <stdio.h>			/* buffered i/o package */
#include <ctype.h>			/* upper/lower case macros */

#ifdef LATTICE
struct tm {
	int	tm_sec;
	int	tm_min;
	int	tm_hour;
	int	tm_mday;
	int	tm_mon;
	int	tm_year;
	int	tm_wday;
	int	tm_yday;
	int	tm_isdst;
};
#endif

char	string[432];
	
/*
 *	Day of week headers.
 */

static	char	dayw[] = { " S  M Tu  W Th  F  S" };

/*
 *	Month of year headers.
 */

static	char	*smon[]= {
	"January",	"February",	"March",	"April",
	"May",		"June",		"July",		"August",
	"September",	"October",	"November",	"December",
};

main(argc, argv)
int	argc;					/* argument count */
char	*argv[];				/* argument vector */
{
	extern	int		exit();		/* terminate our process */
	extern	int		fprintf();	/* print formatted to file */
	extern	int		printf();	/* print formatted */
	extern	long		time();		/* get current system time */
	extern	struct	tm	*localtime();	/* convert sys to local time */

	extern	pstr();				/* print calendar string */

	register int	m;			/* month */
	register int	y;			/* year */
	register int	i;
	register int	j;

	long		systime;		/* system time */
	struct	tm	*local;			/* local time */

	/**
	 *	Get the system time.
	**/
	
	time(&systime);
	
	/**
	 *	Convert it to local time.
	**/
	
	local = localtime(&systime);
	
	/**
	 *	Print the whole year if there was exactly one argument other
	 *	than the invocation name, and that argument is a number greater
	 *	than 13, or if there was no argument.
	**/

	if (argc == 1 || (argc == 2 && (y = number(argv[1])) > 12)) {

		/**
		 *	Print out the current year if
		 *	no arguments are specified.
		**/

		if (argc == 1) {
		
			/**
			 *	Extract the year and adjust it for this century.
			**/

			y = local->tm_year + 1900;

		}
	
		/**
		 *	Get the year from the command line.
		**/

		else {

			/**
			 *	Check for allowable years
			**/

			if (y < 1 || y > 9999) {
				usage();
			}

			/**
			 *	Allow abbreviations: 86 --> 1986.
			**/

			if (y < 100) {
				y += 1900;
			}
		}

		/**
		 *	Print the year header.
		**/

		printf("\n\n\n				%u\n\n", y);

		/**
		 *	Cycle through the months.
		**/

		for (i = 0; i < 12; i += 3) {
			for (j = 0; j < 6 * 72; j++)
				string[j] = '\0';

			printf("         %.3s", smon[i]);
			printf("                    %.3s", smon[i + 1]);
			printf("                    %.3s\n", smon[i + 2]);
			printf("%s   %s   %s\n", dayw, dayw, dayw);

			cal(i + 1, y, string, 72);
			cal(i + 2, y, string + 23, 72);
			cal(i + 3, y, string + 46, 72);

			for (j = 0; j < 6 * 72; j += 72)
				pstr(string + j, 72);
		}
		printf("\n\n\n");
	}

	else {

		/**
		 *	Print the current month if there was exactly one
		 *	argument other than the invocation name, and that
		 *	argument is a number less than 13.
		**/

		if (argc == 2 && (y = number(argv[1])) <= 12) {
		
			/**
			 *	Extract the year and adjust it for this century.
			**/

			y = local->tm_year + 1900;

			/**
			 *	Get the month from the command line.
			**/

			m = number(argv[1]);

			/**
			 *	If the month has already passed, use
			 *	next year.
			**/

			if (m < local->tm_mon+1) {
				y++;
			}

		}

		/**
		 *	Print a specific month from the specified year if
		 *	there was more than one argument other than the
		 *	invocation name.
		**/

		else {
			/**
			 *	Get the month from the command line.
			**/

			m = number(argv[1]);
	
			/**
			 *	Get the year from the command line.  Allow
			 *	abbreviations of form nn -> 19nn.
			**/

			y = number(argv[2]);
			if (y >0 && y < 100) {
				y += 1900;
			}
		}

		/**
		 *	Generate an error if the month is illegal.
		**/

		if (m < 1 || m > 12) {
			fprintf(stderr,
				"cal:  month must be between 1 and 12.\n");
			usage();
		}

		/**
		 *	Generate an error if the year is illegal.
		**/

		if (y < 1 || y > 9999) {
			fprintf(stderr,
				"cal:  year must be between 1 and 9999.\n");
			usage();
		}

		/**
		 *	Print the month and year header.
		**/

		printf("   %s %u\n", smon[m - 1], y);

		/**
		 *	Print the day of week header.
		**/

		printf("%s\n", dayw);

		/**
		 *	Generate the calendar for the month and year.
		**/

		cal(m, y, string, 24);

		/**
		 *	Print out the month.
		**/

		for (i = 0; i < 6 * 24; i += 24)
			pstr(string + i, 24);
	}

	/**
	 *	All done.
	**/

	exit(0);
}

int
number(str)
register char	*str;				/* string to convert */
{
	int		cicmp();		/* case-insensitive compare */

	register int	n;			/* number value of string */
	register char	*s,*p;			/* loop pointers */

	/**
	 *	Convert the string to a number.
	**/

	for (n = 0, s = str; *s >= '0' && *s <= '9'; s++) {
		n = n * 10 + *s - '0';
	}
	
	if (*s == '\0') {
		return (n);
	}

	/**
	 *	If it's not a number, check if it's a month.
	**/

	for (n=0; n<12; n++) {
		if (cicmp(str,smon[n]) == 0) {
			return (n+1);
		}
	}

	/**
	 *	Otherwise, give up and return zero.
	**/
	
	return (0);
}

pstr(str, n)
char	*str;
int	n;
{
	register int	i;
	register char	*s;

	s = str;
	i = n;

	while (i--)
		if (*s++ == '\0')
			s[-1] = ' ';

	i = n + 1;

	while (i--)
		if (*--s != ' ')
			break;

	s[1] = '\0';
	printf("%s\n", str);

	return;
}

cal(m, y, p, w)
int	m;						/* month */
int	y;						/* year */
char	*p;
int	w;
{
	register int	d;
	register int	i;
	register char	*s;

	/*
	 *	Number of days per month table.
	 */

	static	char	mon[] = {
		0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	};

	s = p;

	/**
	 *	Get the day of the week for January 1 of this `y`ear.
	**/

	d = jan1(y);

	/**
	 *	Restore the days-per-month for February and September because
	 *	they may have been fiddled with.
	**/

	mon[2] = 29;
	mon[9] = 30;

	switch ((jan1(y + 1) + 7 - d) % 7) {

		/*
		 *	non-leap year
		 */
	case 1:
		mon[2] = 28;
		break;

		/*
		 *	1752
		 */
	default:
		mon[9] = 19;
		break;

		/*
		 *	leap year
		 */
	case 2:
		;
	}

	for (i = 1; i < m; i++)
		d += mon[i];
	d %= 7;
	s += 3 * d;
	for (i = 1; i <= mon[m]; i++) {
		if (i == 3 && mon[m] == 19) {
			i += 11;
			mon[m] += 11;
		}
		if (i > 9)
			*s = i / 10 + '0';
		s++;
		*s++ = i % 10 + '0';
		s++;
		if (++d == 7) {
			d = 0;
			s = p + w;
			p = s;
		}
	}
}

jan1(y)
register int	y;					/* year */
{
	register int	d;				/* day */

	/**
	 *	Compute the number of days until the first of this year using
	 *	the normal Gregorian calendar which has one extra day per four
	 *	years.
	**/

	d = 4 + y + (y + 3) / 4;

	/**
	 *	Adjust for the Julian and Regular Gregorian calendars which
	 *	have three less days per each 400.
	**/

	if (y > 1800) {
		d -= (y - 1701) / 100;
		d += (y - 1601) / 400;
	}

	/**
	 *	Add three days if necessary to account for the great
	 *	calendar changeover instant.
	**/

	if (y > 1752)
		d += 3;
	
	/**
	 *	Get the day of the week from the day count.
	**/

	return (d % 7);
}

usage()
{
	fprintf(stderr,"Usage:\tcal [m] [y]\n");
	fprintf(stderr,"\t'm' is 1 thru 12 or any reasonable month name.\n");
	fprintf(stderr,"\t'y' is a year between 100 and 9999.\n");
	fprintf(stderr,"\tYears 13-99 are abbreviations for 1913-1999.\n");
	fprintf(stderr,"\tWith no arguments, the current year is printed.\n");
	fprintf(stderr,"\tWith only a month given, the next instance of\n");
	fprintf(stderr,"\t\tthat month (this year or next) is printed.\n");
	fprintf(stderr,"\tYear as a single argument gives that whole year.\n");

	exit(-1);
}

int cicmp(s,pat)
char *s,*pat;
{
	char	c1,c2;
	while (*s) {
		c1 = *s++;
		c2 = *pat++;
		if (isupper(c1)) {
			c1 = tolower(c1);
		}
		if (isupper(c2)) {
			c2 = tolower(c2);
		}
		if (c1 != c2) {
			return (1);
		}
	}
	return (0);
}

#ifdef LATTICE
long time()
{
}

struct tm *localtime()
{
	extern long time_bin(),date_bin();
	long date,time;
	static struct tm t;

	time=time_bin();
	date=date_bin();

	t.tm_year=(date >> 16) & 0x0000FFFFL;
	t.tm_year -= 1900;	/* conform to unix */
	t.tm_mon=(date >> 8) & 0x000000FFL;
	t.tm_mon -= 1;		/* unix months start at zero */
	t.tm_day=date & 0x000000FFL;
	t.tm_hr=(time >> 24) & 0x000000FFL;
	t.tm_min=(time >> 16) & 0x000000FFL;
	t.tm_sec=(time >> 8) & 0x000000FFL;

	return (&t);
}
#endif

