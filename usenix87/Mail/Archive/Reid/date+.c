/* date+ - add specified time to current date */

/* Please send additions, bug fixes, portifications, etc. to:
 *
 *	Daniel LaLiberte
 *	ihnp4!uiucdcs!liberte
 *	University of Illinois, Urbana-Champaign
 *	Department of Computer Science
 */

/* This is written for BSD42 */

#include <stdio.h>
#include <sys/time.h>

long tloc;
long time();

char *ctime();
struct tm *localtime();

struct tm *ts;

double atof();


int argc;	/* global argument passing */
char **argv;



main (Argc, Argv)
/* return time + 1st arg hours */

int Argc;
char *Argv[];

{
	argc = Argc;
	argv = Argv;
	incrdate();
	printdate();

} /* main */



incrdate()	/* increment date from arguments */
{
	static char *unit[] =
	{
		"sec", "min", "hour", "day", "week", "mon", "year", ""	};

	static int conv[] =	/* conversion factor */
	{
		1, 60, 3600, 86400, 604800, 0, 0	};

	int i;
	double value;
	long total;	/* cummulative total increment in whole seconds */
	double monthincr = 0.0,	/* store increment of month and year */
	yearincr = 0.0; 	/* since months and years are not uniform */

	time(&tloc);	/* current time */
	argc--; 
	argv++;

	while (argc &&
	    ((**argv == '.') ||
	    (**argv == '-') || 
	    (**argv == '+') || 
	    (**argv >= '0' && **argv <= '9'))) {
		value = atof(argv[0]);
/*		printf("%s = %f", argv[0], value); */

		argv++; 
		argc--;
		if (argc == 0) missing();
		else {	/* search for unit */
			for (i = 0;	(i < 7) &&
				(0 != strncmp(argv[0], unit[i], 
					strlen(unit[i])));)
				i++;
			if (i == 7) missing();
			else { 
				argv++; 
				argc--;
				if (i < 5) value *= conv[i];
				if (i == 5) monthincr += value;
				if (i == 6) yearincr += value;
			}
/*			printf(" %s (%f seconds)\n", unit[i], value); */
		}

		total += value;
	}


	tloc += total;
	ts = localtime(&tloc);
	ts->tm_mon += monthincr;
	ts->tm_year += yearincr;
	if (ts->tm_mon >= 12) {
		ts->tm_year += (ts->tm_mon + 1) / 12;
		ts->tm_mon = ts->tm_mon % 12;
	}
 	else if (ts->tm_mon < 0) {
		ts->tm_year += (ts->tm_mon + 1 - 12) / 12;
		ts->tm_mon = ((ts->tm_mon % 12) + 12) % 12;
	}
} /* getincr */


missing()
{
	fprintf(stderr, "date+: missing unit\n");
	exit (1);
} 


printdate()
{
	char *format;

	static char *month[] = 
	    {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"	};

	static char *day[] =
	    {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"	};


	if (argc == 0) {	/* put default format on argv */
		argv[0] = "%h:%m %N %D";
		argc++;
	}

	while (argc > 0) {
		format = argv[0];

		while (*format) {
			if (*format != '%')
				putchar(*format);
			else if (format[1]) 

				switch(*++format) {
				case 's': 
					printf("%02d", ts->tm_sec);  
					break;
				case 'S': 
					printf("%d", tloc);  
					break;
				case 'm': 
					printf("%02d", ts->tm_min);  
					break;
				case 'h': 
					printf("%02d", ts->tm_hour);  
					break;
				case 'd': 
					printf("%02d", ts->tm_mday);  
					break;
				case 'D': 
					printf("%d", ts->tm_mday);  
					break;
				case 'n': 
					printf("%02d", ts->tm_mon + 1);  
					break;
				case 'N': 
					printf("%s", month[ts->tm_mon]);  
					break;
				case 'y': 
					printf("%02d", ts->tm_year);  
					break;
				case 'w': 
					printf("%1d", ts->tm_wday + 1);  
					break;
				case 'W': 
					printf("%s", day[ts->tm_wday]);  
					break;
				case 'Y': 
					printf("%d", ts->tm_yday);  
					break;

				default:  
					fprintf(stderr, "date+: Bad format character: '%c'\n", *format); 
					exit(1);
				}

			format++;
		} /* while (*format) */

		argc--; 
		argv++;
		if (argc > 0) 
			putchar(' ');
	} /* while (argc > 0) */

	putchar('\n');
} /* printdate */
