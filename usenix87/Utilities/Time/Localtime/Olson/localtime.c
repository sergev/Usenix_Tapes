#

/*LINTLIBRARY*/

/*
** sys/types.h is included to get time_t.
*/

#include "sys/types.h"
#include "tzfile.h"
#include "time.h"

#ifndef MAXPATHLEN
#include "sys/param.h"
#ifndef MAXPATHLEN
#define MAXPATHLEN	1024
#endif /* !MAXPATHLEN */
#endif /* !MAXPATHLEN */

#ifndef lint
#ifndef NOID
static char	sccsid[] = "@(#)localtime.c	3.1";
#endif /* !NOID */
#endif /* !lint */

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif /* !TRUE */

extern char *		getenv();
extern char *		strcpy();
extern char *		strcat();
#ifdef STD_INSPIRED
struct tm *		offtime();
#else /* !STD_INSPIRED */
static struct tm *	offtime();
#endif /* !STD_INSPIRED */

struct ttinfo {				/* time type information */
	long		tt_gmtoff;	/* GMT offset in seconds */
	int		tt_isdst;	/* used to set tm_isdst */
	int		tt_abbrind;	/* abbreviation list index */
};

struct state {
	int		timecnt;
	int		typecnt;
	int		charcnt;
	time_t		ats[TZ_MAX_TIMES];
	unsigned char	types[TZ_MAX_TIMES];
	struct ttinfo	ttis[TZ_MAX_TYPES];
	char		chars[TZ_MAX_CHARS + 1];
};

static struct state	s;

static int		tz_is_set;

char *			tzname[2] = {
	"GMT",
	"GMT"
};

#ifdef USG_COMPAT
time_t			timezone = 0;
int			daylight = 0;
#endif /* USG_COMPAT */

#ifdef TZA_COMPAT
char *			tz_abbr;	/* compatibility w/older versions */
#endif /* TZA_COMPAT */

static long
detzcode(codep)
char *	codep;
{
	register long	result;
	register int	i;

	result = 0;
	for (i = 0; i < 4; ++i)
		result = (result << 8) | (codep[i] & 0xff);
	return result;
}

static
tzload(name)
register char *	name;
{
	register int	i;
	register int	fid;

	if (name == 0 && (name = TZDEFAULT) == 0)
		return -1;
	{
		register char *	p;
		register int	doaccess;
		char		fullname[MAXPATHLEN];

		doaccess = name[0] == '/';
		if (!doaccess) {
			if ((p = TZDIR) == 0)
				return -1;
			if ((strlen(p) + strlen(name) + 1) >= sizeof fullname)
				return -1;
			(void) strcpy(fullname, p);
			(void) strcat(fullname, "/");
			(void) strcat(fullname, name);
			/*
			** Set doaccess if '.' (as in "../") shows up in name.
			*/
			while (*name != '\0')
				if (*name++ == '.')
					doaccess = TRUE;
			name = fullname;
		}
		if (doaccess && access(name, 4) != 0)
			return -1;
		if ((fid = open(name, 0)) == -1)
			return -1;
	}
	{
		register char *			p;
		register struct tzhead *	tzhp;
		char				buf[sizeof s];

		i = read(fid, buf, sizeof buf);
		if (close(fid) != 0 || i < sizeof *tzhp)
			return -1;
		tzhp = (struct tzhead *) buf;
		s.timecnt = (int) detzcode(tzhp->tzh_timecnt);
		s.typecnt = (int) detzcode(tzhp->tzh_typecnt);
		s.charcnt = (int) detzcode(tzhp->tzh_charcnt);
		if (s.timecnt > TZ_MAX_TIMES ||
			s.typecnt == 0 ||
			s.typecnt > TZ_MAX_TYPES ||
			s.charcnt > TZ_MAX_CHARS)
				return -1;
		if (i < sizeof *tzhp +
			s.timecnt * (4 + sizeof (char)) +
			s.typecnt * (4 + 2 * sizeof (char)) +
			s.charcnt * sizeof (char))
				return -1;
		p = buf + sizeof *tzhp;
		for (i = 0; i < s.timecnt; ++i) {
			s.ats[i] = detzcode(p);
			p += 4;
		}
		for (i = 0; i < s.timecnt; ++i)
			s.types[i] = (unsigned char) *p++;
		for (i = 0; i < s.typecnt; ++i) {
			register struct ttinfo *	ttisp;

			ttisp = &s.ttis[i];
			ttisp->tt_gmtoff = detzcode(p);
			p += 4;
			ttisp->tt_isdst = (unsigned char) *p++;
			ttisp->tt_abbrind = (unsigned char) *p++;
		}
		for (i = 0; i < s.charcnt; ++i)
			s.chars[i] = *p++;
		s.chars[i] = '\0';	/* ensure '\0' at end */
	}
	/*
	** Check that all the local time type indices are valid.
	*/
	for (i = 0; i < s.timecnt; ++i)
		if (s.types[i] >= s.typecnt)
			return -1;
	/*
	** Check that all abbreviation indices are valid.
	*/
	for (i = 0; i < s.typecnt; ++i)
		if (s.ttis[i].tt_abbrind >= s.charcnt)
			return -1;
	/*
	** Set tzname elements to initial values.
	*/
	tzname[0] = tzname[1] = &s.chars[0];
#ifdef USG_COMPAT
	timezone = s.ttis[0].tt_gmtoff;
	daylight = 0;
#endif /* USG_COMPAT */
	for (i = 1; i < s.typecnt; ++i) {
		register struct ttinfo *	ttisp;

		ttisp = &s.ttis[i];
		if (ttisp->tt_isdst) {
			tzname[1] = &s.chars[ttisp->tt_abbrind];
#ifdef USG_COMPAT
			daylight = 1;
#endif /* USG_COMPAT */ 
		} else {
			tzname[0] = &s.chars[ttisp->tt_abbrind];
#ifdef USG_COMPAT
			timezone = ttisp->tt_gmtoff;
#endif /* USG_COMPAT */ 
		}
	}
	return 0;
}

static
tzsetgmt()
{
	s.timecnt = 0;
	s.ttis[0].tt_gmtoff = 0;
	s.ttis[0].tt_abbrind = 0;
	(void) strcpy(s.chars, "GMT");
	tzname[0] = tzname[1] = s.chars;
#ifdef USG_COMPAT
	timezone = 0;
	daylight = 0;
#endif /* USG_COMPAT */
}

void
tzset()
{
	register char *	name;

	tz_is_set = TRUE;
	name = getenv("TZ");
	if (name != 0 && *name == '\0')
		tzsetgmt();		/* GMT by request */
	else if (tzload(name) != 0)
		tzsetgmt();
}

void
tzsetwall()
{
	tz_is_set = TRUE;
	if (tzload((char *) 0) != 0)
		tzsetgmt();
}

struct tm *
localtime(timep)
time_t *	timep;
{
	register struct ttinfo *	ttisp;
	register struct tm *		tmp;
	register int			i;
	time_t				t;

	if (!tz_is_set)
		(void) tzset();
	t = *timep;
	if (s.timecnt == 0 || t < s.ats[0]) {
		i = 0;
		while (s.ttis[i].tt_isdst)
			if (++i >= s.timecnt) {
				i = 0;
				break;
			}
	} else {
		for (i = 1; i < s.timecnt; ++i)
			if (t < s.ats[i])
				break;
		i = s.types[i - 1];
	}
	ttisp = &s.ttis[i];
	/*
	** To get (wrong) behavior that's compatible with System V Release 2.0
	** you'd replace the statement below with
	**	tmp = offtime((time_t) (t + ttisp->tt_gmtoff), 0L);
	*/
	tmp = offtime(&t, ttisp->tt_gmtoff);
	tmp->tm_isdst = ttisp->tt_isdst;
	tzname[tmp->tm_isdst] = &s.chars[ttisp->tt_abbrind];
#ifdef KRE_COMPAT
	tmp->tm_zone = &s.chars[ttisp->tt_abbrind];
#endif /* KRE_COMPAT */
#ifdef TZA_COMPAT
	tz_abbr = &s.chars[ttisp->tt_abbrind];
#endif /* TZA_COMPAT */
	return tmp;
}

struct tm *
gmtime(clock)
time_t *	clock;
{
	register struct tm *	tmp;

	tmp = offtime(clock, 0L);
	tzname[0] = "GMT";
#ifdef KRE_COMPAT
	tmp->tm_zone = "GMT";		/* UCT ? */
#endif /* KRE_COMPAT */
	return tmp;
}

static int	mon_lengths[2][MONS_PER_YEAR] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int	year_lengths[2] = {
	DAYS_PER_NYEAR, DAYS_PER_LYEAR
};

#ifdef STD_INSPIRED
struct tm *
#else /* !STD_INSPIRED */
static struct tm *
#endif /* !STD_INSPIRED */
offtime(clock, offset)
time_t *	clock;
long		offset;
{
	register struct tm *	tmp;
	register long		days;
	register long		rem;
	register int		y;
	register int		yleap;
	register int *		ip;
	static struct tm	tm;

	tmp = &tm;
	days = *clock / SECS_PER_DAY;
	rem = *clock % SECS_PER_DAY;
	rem += offset;
	while (rem < 0) {
		rem += SECS_PER_DAY;
		--days;
	}
	while (rem >= SECS_PER_DAY) {
		rem -= SECS_PER_DAY;
		++days;
	}
	tmp->tm_hour = (int) (rem / SECS_PER_HOUR);
	rem = rem % SECS_PER_HOUR;
	tmp->tm_min = (int) (rem / SECS_PER_MIN);
	tmp->tm_sec = (int) (rem % SECS_PER_MIN);
	tmp->tm_wday = (int) ((EPOCH_WDAY + days) % DAYS_PER_WEEK);
	if (tmp->tm_wday < 0)
		tmp->tm_wday += DAYS_PER_WEEK;
	y = EPOCH_YEAR;
	if (days >= 0)
		for ( ; ; ) {
			yleap = isleap(y);
			if (days < (long) year_lengths[yleap])
				break;
			++y;
			days = days - (long) year_lengths[yleap];
		}
	else do {
		--y;
		yleap = isleap(y);
		days = days + (long) year_lengths[yleap];
	} while (days < 0);
	tmp->tm_year = y - TM_YEAR_BASE;
	tmp->tm_yday = (int) days;
	ip = mon_lengths[yleap];
	for (tmp->tm_mon = 0; days >= (long) ip[tmp->tm_mon]; ++(tmp->tm_mon))
		days = days - (long) ip[tmp->tm_mon];
	tmp->tm_mday = (int) (days + 1);
	tmp->tm_isdst = 0;
#ifdef KRE_COMPAT
	tmp->tm_zone = "";
	tmp->tm_gmtoff = offset;
#endif /* KRE_COMPAT */
	return tmp;
}

#ifdef BSD_COMPAT

/*
** If ctime and localtime aren't in the same file on 4.3BSD systems,
** you can run into compilation problems--take
**	cc date.c -lz
** (please).
*/

char *
ctime(timep)
time_t *	timep;
{
	return asctime(localtime(timep));
}

#endif /* BSD_COMPAT */
