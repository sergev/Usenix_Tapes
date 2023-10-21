/* date+ - add specified time to current date */

/* Please send additions, bug fixes, portifications, etc. to:
 *
 *	Daniel LaLiberte
 *	ihnp4!uiucdcs!liberte
 *	University of Illinois, Urbana-Champaign
 *	Department of Computer Science
 */

/* This is written for BSD42 */

/* This was revised for SYSTEM V by:
 *
 *	Robert O. Domitz
 *	...!vax135!petsd!pecnos!rod
 *	Concurrent Computer Corporation
 *	106 Apple Street
 *	Tinton Falls, NJ  07724
 */

#include <stdio.h>
#include <time.h>

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
		argv[0] = "%H:%M %h %d";
		argc++;
	}

	while (argc > 0) {
		format = argv[0];

		while (*format) {
			if (*format != '%')
				putchar(*format);
			else if (format[1]) 

				switch(*++format) {
				case 'n':
					putchar ('\n');
					break;
				case 't':
					putchar ('\t');
					break;
				case 'S': 
					printf("%02d", ts->tm_sec);  
					break;
				case 'Z': 
					printf("%d", tloc);  
					break;
				case 'M': 
					printf("%02d", ts->tm_min);  
					break;
				case 'H': 
					printf("%02d", ts->tm_hour);  
					break;
				case 'T':
					printf("%02d:%02d:%02d",
						ts->tm_hour, ts->tm_min,
						ts->tm_sec);
					break;
				case 'd': 
					printf("%02d", ts->tm_mday);  
					break;
/*				case 'D': 
					printf("%d", ts->tm_mday);  
					break;			*/
				case 'm': 
					printf("%02d", ts->tm_mon + 1);  
					break;
				case 'h': 
					printf("%s", month[ts->tm_mon]);  
					break;
				case 'y': 
					printf("%02d", ts->tm_year);  
					break;
				case 'w': 
					printf("%1d", ts->tm_wday);  
					break;
				case 'a': 
					printf("%s", day[ts->tm_wday]);  
					break;
				case 'D':
					printf("%02d/%02d/%02d",
						ts->tm_mon + 1, ts->tm_mday,
						ts->tm_year);
					break;
				case 'j': 
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
