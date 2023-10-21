static char sccsid[] = "@(#)fdate.c	4.3 (Melbourne) 82/01/25";

/*
 *	fdate - formatted date output
 */

#include        <stdio.h>
#include	<sys/types.h>
#include	<time.h>

#ifdef SysV
struct timeb {
	time_t	time;
	int	millitm;
	int	timezone;
};
#else
#include	<sys/timeb.h>
#endif

#define	MONTH	itoa(tim->tm_mon+1,p)
#define	DAY	itoa(tim->tm_mday,p)
#define	YEAR	itoa(tim->tm_year,p)
#define	HOUR	itoa(tim->tm_hour,p)
#define	MINUTE	itoa(tim->tm_min,p)
#define	SECOND	itoa(tim->tm_sec,p)
#define	JULIAN	itoa(tim->tm_yday+1,p)
#define	WEEKDAY	itoa(tim->tm_wday,p)
#define	MODHOUR	itoa(h,p)
#define	MILISEC	itoa(timeb.millitm,p)

#define	FILL(num) {if (fill == '0' && tim->tm_/**/num < 10) *p++ = '0';}

char	*month[] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
};

int	monthdays[12] = {
	31,
	28,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
	31
};

char	*days[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
};

char	*suffix[] = {
	"th",
	"st",
	"nd",
	"rd",
	"th",
	"th",
	"th",
	"th",
	"th",
	"th",
};

char	*itoa();
#ifndef SysV
char	*timezone();
#endif
struct	tm	*localtime();
char	*asctime();
int	nonl;

struct	tm	incr	= { 0 };



main(argc, argv)
	int	argc;
	char	**argv;
{
	register char	*aptr, c;
	struct	timeb	timeb;
	register struct	tm  *tim;
	register int i, h, hflag;
	char *p;
	int	ndig, nchar, ljust;
	char	fill;
	char	lead;
	int	hadincr;
	int	ac;
	char	**av;
	char	 buf[60];
	extern	char	_sobuf[];

	setbuf(stdout, _sobuf);

#ifdef SysV
	tzset();
#endif

	ftime(&timeb);
	tim = localtime(&timeb.time);

	lead = '%';

	while (argc > 1 && argv[1][0] == '-') {
		switch (argv[1][1]) {

		case 'n':
			if (argv[1][2] == '\0') {
				nonl++;
				argv++;
				argc--;
				continue;
			}
			break;

		case 'F':
			if (argv[1][2] != '\0' && argv[1][3] == '\0') {
				lead = argv[1][2];
				argv++;
				argc--;
				continue;
			}
			break;
		}
		break;
	}

	ac = argc;
	av = argv;

	hadincr = 0;
	while (--ac > 0) {
		aptr = *++av;

		while (c = *aptr++) {
			if (c != lead)
				continue;

			i = 0;

			if (*aptr == '-')
				aptr++;
			while (*aptr >= '0' && *aptr <= '9')
				aptr++;
			if (*aptr == '.')
				while (*++aptr >= '0' && *aptr <= '9')
					;
			if (*aptr == '+') {
				while (*++aptr >= '0' && *aptr <= '9')
					i *= 10, i += *aptr - '0';
				if (*aptr == '+')
					aptr++;
			}
			hadincr++;

			switch (*aptr++) {

			case 'd':	incr.tm_mday = i;	break;
			case 'm':	incr.tm_mon = i;	break;
			case 'y':	incr.tm_year = i;	break;

			case 'H':	incr.tm_hour = i;	break;
			case 'M':	incr.tm_min = i;	break;
			case 'S':	incr.tm_sec = i;	break;

			case '\0':	aptr--;		/* fall through */
			default:	hadincr--;		break;
			}
		}
	}

	if (hadincr) {
		tim->tm_sec += incr.tm_sec;
		while (tim->tm_sec >= 60) {
			tim->tm_min++;
			tim->tm_sec -= 60;
		}

		tim->tm_min += incr.tm_min;
		while (tim->tm_min >= 60) {
			tim->tm_hour++;
			tim->tm_min -= 60;
		}

		tim->tm_hour += incr.tm_hour;
		while (tim->tm_hour >= 24) {
			tim->tm_mday++;
			tim->tm_hour -= 24;
		}

		setyear(tim->tm_year);
		tim->tm_mday += incr.tm_mday;
		while (tim->tm_mday > monthdays[tim->tm_mon]) {
			tim->tm_mday -= monthdays[tim->tm_mon];
			if (++tim->tm_mon >= 12) {
				tim->tm_mon -= 12;
				setyear(++tim->tm_year);
			}
		}

		tim->tm_mon += incr.tm_mon;
		while (tim->tm_mon >= 12) {
			tim->tm_year++;
			tim->tm_mon -= 12;
		}

		tim->tm_year += incr.tm_year;
		setyear(tim->tm_year);

		if (tim->tm_mday > monthdays[tim->tm_mon])
			tim->tm_mday = monthdays[tim->tm_mon];

		timeb.time = 0;
		tim->tm_year += 1900;

		for (i = 1970; i < tim->tm_year; i++)
			timeb.time += dysize(i);

		while (tim->tm_mon)
			timeb.time += monthdays[--tim->tm_mon];

		timeb.time += tim->tm_mday - 1;
		timeb.time *= 24;
		timeb.time += tim->tm_hour;
		timeb.time *= 60;
		timeb.time += tim->tm_min;
		timeb.time *= 60;
		timeb.time += tim->tm_sec;

		timeb.time += (long)timeb.timezone*60;
		if (tim->tm_isdst)
			timeb.time -= 60*60;
		tim = localtime(&timeb.time);
	}

	while (--argc > 0) {
		aptr = *++argv;

		while (c = *aptr++) {
			for (p = buf; p < &buf[60]; )
				*p++ = '\0';

			ndig = nchar = ljust = 0;
			fill = ' ';
			p = buf;
			if (c == lead) {
				if (*aptr == '-') {
					aptr++;
					ljust++;
				}
				if (*aptr == '0') {
					fill = '0';
					aptr++;
				}
				i = 0;
				while (*aptr >= '0' && *aptr <= '9') {
					i *= 10;
					i += *aptr++ - '0';
				}
				ndig = i;
				if (*aptr == '.') {
					i = 0;
					while (*++aptr >= '0' && *aptr <= '9')
						i *= 10, i += *aptr - '0';
					nchar = i;
				}
				if (*aptr == '+') {
					while (*++aptr >= '0' && *aptr <= '9')
						;
					if (*aptr == '+') {
						aptr++;
						if (*aptr)
							aptr++;
						continue;
					}
				}
				switch (c = *aptr++) {
				case '%':
					putchar('%');
					continue;
				case 'n':
					putchar('\n');
					continue;
				case 't':
					putchar('\t');
					continue;
				case 'p':
					if (tim->tm_hour >= 12)
						p = "pm";
					else
						p = "am";
					strcpy(buf, p);
					break;
				case 'P':
					if (tim->tm_hour >= 12)
						p = "PM";
					else
						p = "AM";
					strcpy(buf, p);
					break;
				case 'h':
					i = tim->tm_mday;
					if (i > 10 && i < 19)
						i = 5;
					i %= 10;
					strcpy(buf, suffix[i]);
					break;
				case 'm' :
					p = MONTH;
					break;
				case 'd':
					p = DAY;
					break;
				case 'y' :
					p = YEAR;
					break;
				case 'Y':
					FILL(year);
					p = YEAR;
					*p++ = '/';
					FILL(mon+1);
					p = MONTH;
					*p++ = '/';
					FILL(mday);
					p = DAY;
					break;
				case 'D':
					FILL(mday);
					p = DAY;
					*p++ = '/';
					FILL(mon+1);
					p = MONTH;
					*p++ = '/';
					FILL(year);
					p = YEAR;
					break;
				case 'H':
					p = HOUR;
					break;
				case 'M':
					p = MINUTE;
					break;
				case 'S':
					p = SECOND;
					break;
				case 'T':
					FILL(hour);
					p = HOUR;
					*p++ = ':';
					FILL(min);
					p = MINUTE;
					*p++ = ':';
					FILL(sec);
					p = SECOND;
					break;
				case 'j':
					p = JULIAN;
					break;
				case 'w':
					p = WEEKDAY;
					break;
				case 'R':
					h = tim->tm_hour;
					if ((h %= 12) == 0)
						h = 12;
					p = MODHOUR;
					break;
				case 'r':
					if ((h = tim->tm_hour) >= 12)
						hflag++;
					if ((h %= 12) == 0)
						h = 12;
					if (fill == '0' && h < 10)
						*p++ = '0';
					p = MODHOUR;
					*p++ = ':';
					FILL(min);
					p = MINUTE;
					*p++ = ':';
					FILL(sec);
					p = SECOND;
					*p++ = ' ';
					if (hflag)
						*p++ = 'P';
					else 
						*p++ = 'A';
					*p++ = 'M';
					break;
				case 'l':
					if (nchar == 0 || nchar > 3)
						nchar = 3;
				case 'L':
					p = month[tim->tm_mon];
					strcpy(buf, p);
					break;
				case 'a':
					if (nchar == 0 || nchar > 3)
						nchar = 3;
				case 'A':
					p = days[tim->tm_wday];
					strcpy(buf, p);
					break;
				case 'X':
					p = MILISEC;
					break;
				case 'C':
					if (nchar == 0 || nchar > 24)
						nchar = 24;
					p = asctime(tim);
					strcpy(buf, p);
					break;
				case 'z':
#ifdef SysV
					p = tzname[tim->tm_isdst];
#else
					p = timezone(timeb.timezone,tim->tm_isdst);
#endif
					strcpy(buf, p);
					break;
				default:
					if (c == lead) {
						putchar(c);
						break;
					}
					if (c == '\0' || c == ' ')
						fprintf(stderr, "fdate: missing format character in \"%s\"\n", *argv);
					else
						fprintf(stderr, "fdate: bad format character - %c\n", c);
					close(1);	/* trash stdout! */
					exit(2);

				}	/* endsw */
			} else {
				putchar(c);
				continue;
			}

			h = strlen(buf);
			if (nchar && nchar < h)
				h = nchar;
			i = ndig - h;
			if (!ljust)
				while (i-- > 0)
					putchar(fill);
			p = buf;
			while (h-- > 0)
				putchar(*p++);
			while (i-- > 0)
				putchar(fill);

		}	/* endwh */

		if (argc > 1)
			putchar(' ');
	}	/* endwh */

	if (!nonl)
		putchar('\n');
	exit(0);
}

char *
itoa(i, ptr)
	register  int	i;
	register  char	*ptr;
{
	char	buf[8];
	register	char *p = buf;
	do {
		*p++ = i % 10 + '0';
		i /= 10;
	} while(i);

	while (p > buf)
		*ptr++ = *--p;
	*ptr = '\0';
	return (ptr);
}

setyear(yr)
	register yr;
{
	if (yr > 2100) {
		fprintf(stderr, "fdate: time got too big\n");
		exit(1);
	}

	if (dysize(yr) == 366)
		monthdays[1] = 29;
	else
		monthdays[1] = 28;
}

#ifdef SysV

ftime(p)
	struct timeb *p;
{
	(void) time (&p->time);
	p->millitm = 0;
	p->timezone = timezone / 60;
}

int
dysize(yr)
	register yr;
{
	if (yr % 4)
		return (365);
	if (yr % 100)
		return (366);
	if (yr % 400)
		return (365);
	return (366);
}

#endif
