#include <stdio.h>
#include <ctype.h>

#define BSD42
#ifdef BSD42
#include <sys/time.h>
#else
#include <time.h>
#endif
#define MAXTOK 20
/*
 * routines to turn a date from various formats to other formats
 *
 *	Primary interesting routine is gdate() which eats a date
 *	string of several common formats (see comment) and
 *	fills in a standard UNIX tm structure.
 **	Barry Shein, Boston University
 *      hacked further by sundar , mit a.i.lab
 *	if you compile it -DDEBUG (with DEBUG defined) it will
 *	pull in a main() routine to run standalone for testing.
 */
#define AMBIG  -1	/* ambiguous month */
#define FALSE  -2	/* bad syntax	   */

static char *months[] = {
	"january", "february", "march", "april", "may", "june", "july",
	"august", "september", "october", "november", "december", 0
} ;

static char *dow[] = {
	"sunday", "monday", "tuesday", "wednesday", "thursday",
	"friday", "saturday", 0
} ;

/*
 *	known time-zone name list
 */
static char *tz[] = 
{
	"adt", "ast", "edt", "est", "cdt", "cst", "mst", "mdt",
	"pdt", "pst", "gmt",
	0
} ;

static char mdays[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
} ;

static lcase(c)
register char c ;
{
	return(isupper(c) ? tolower(c) : c) ;
}

/*
 *  get a digit string from sp into bp (buffer) returning new sp
 */
static char *
dstring(bp,sp) 
register char *sp, *bp  ;
{
	register int i = 0 ;
	while(isdigit(*sp))
		if(i++ == MAXTOK) break ;
		else *bp++ = *sp++ ;
	*bp = '\0' ;
	return(sp) ;
}
/*
 *  skip white space returning updated ptr
 */
char *
skipw(sp) 
register char *sp ;
{
	while(isspace(*sp) || *sp == '-') ++sp ;
	return(sp) ;
}

/*
 *  return how many chars of s1 prefix s2
 */
static prefix(s1,s2) 
register char *s1, *s2 ;
{
	register int i = 0  ;

	for(; *s1 == *s2 ; s1++,s2++,i++)
		if(*s2 == '\0') break ;
	return(*s1 == '\0' ? i : 0) ;
}

/*
 *  look up str in list for non-ambiguous (prefix) match
 */
static find(sp,lp) 
register char *sp, **lp ;
{
	int i,j,ambig = 0 ;
	register int k ;
	int ret  = FALSE ;

	for(i = 0,k = 0 ; *lp ; lp++,k++)
		if((j  = prefix(sp,*lp)) > i)
		{
			ambig = 0 ;
			i = j ;
			ret = k + 1 ;
		}
		else if(j && (i == j)) ++ambig ;
	return(ambig ? AMBIG : ret) ;
}

/*
 * like find but demands exact match
 */
static lookup(sp,lp) 
register char *sp, **lp ;
{
	int i = 0 ;

	for(i=0 ; *lp ; lp++,i++)
		if(strcmp(sp,*lp) == 0) return(i+1) ;
	return(0) ;
}

/*
 * extract a token
 */
static char *
extract(bp,sp) 
register char *bp, *sp ;
{
	register int i = 0 ;

	sp = skipw(sp) ;
	for(; isalnum(*sp); sp++)
		if(i++ == MAXTOK) break ;
		else *bp++ = lcase(*sp) ;
	*bp = '\0' ;
	return(sp) ;
}

/*
 *  fill up an area with a value (eg. zeros)
 */
static fill(cp,c,n) 
register char *cp, c; 
register int n;
{
	while(n--) *cp++ = c ;
}

char *gdate(), *gtime() ;
long totime() ;
#if DEBUG
/*
 *  test driver
 *	translates first arg from command line (argv[1])
 *	and dumps out structure built.
 */
usage(sp) 
char *sp;
{
	fprintf(stderr,"Usage: %s date\n",sp) ;
	exit(1) ;
}

main(argc,argv) 
int argc; 
char **argv;
{
	char *cp ;
	struct tm tm ;
	long t,t2 ;

	if(argc != 2) usage(*argv) ;
	if((cp = gdate(*++argv,&tm)) != NULL)
		printf("error: %s (%s)\n",*argv,cp) ;
	printf("year : %d month: %d day: %d\n",
		tm.tm_year,tm.tm_mon,tm.tm_mday);
	printf("day of month: %d hour: %d minute: %d second: %d\n",
		tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec) ;
	printf("day of year: %d day of week: %d dst: %d\n",
		tm.tm_yday, tm.tm_wday, tm.tm_isdst) ;
	time(&t) ;
	t2 = totime(&tm) ;
	printf("%D %D %D\n",t,t2,t2-t) ;
	exit(0) ;
}
#endif	/* DEBUG */

/*
 *	gdate(date_str_ptr,time_buf_ptr)
 *	(see CTIME(3) in UPM for second arg format)
 *	takes many reasonable date strings and translates to
 *	the time-buf structure (integers)
 *
 *	formats (eg.):
 *		oct 19, 1983
 *		OcT 19, 1983 12:43
 *		oCt 19, 1983 12:43:33
 *		oct 19 1983 ...
 *		19 oct 83 ....
 *		10/19/83 12:43:33
 *		19-OCT-83 12:43:00 (DEC style)
 *		Wed Oct 19 12:43 EST 83 (UNIX style)
 *		oct. 19, 83 1:44 pm est
 *		831019124333	(see date(1))
 *	some variations on those themes also.
 *
 *	BUGS: probably a few (maybe some in the eye of the beholder)
 *		does not set dst flag (unless 'EDT' is in string)
 */
char *
gdate(sp,tp) register char *sp ; register struct tm *tp ;
{
	char buf[MAXTOK] ;

	fill(tp,'\0',sizeof *tp) ;
	sp = skipw(sp) ;
	if(isdigit(*sp))	/* either '7/12/83' or '12 jul 83' */
	{
		if(isdigit(sp[1]) && isdigit(sp[2])) /* UNIX yymmddhhmmss */
		{
			buf[2] = '\0' ;
			strncpy(buf,sp,2) ;
			tp->tm_year = atoi(buf) ;
			strncpy(buf,sp += 2,2) ;
			tp->tm_mon = atoi(buf) ;
			sp += 2 ;
			if(!isdigit(*sp)) goto badday ;
			strncpy(buf,sp,2) ;
			tp->tm_mday = atoi(buf) ;
			sp += 2 ;
			if(!isdigit(*sp)) goto check ;
			strncpy(buf,sp,2) ;
			tp->tm_hour ;
			sp += 2 ;
			if(!isdigit(*sp)) goto check ;
			strncpy(buf,sp,2) ;
			tp->tm_min = atoi(buf) ;
			sp += 2 ;
			if(!isdigit(*sp)) goto check ;
			strncpy(buf,sp,2) ;
			tp->tm_min = atoi(buf) ;
			goto check ;
		}
		sp = dstring(buf,sp) ;
		sp = skipw(sp) ;
		if(*sp == '/')	/* must be '7/12/83' */
		{
			if((tp->tm_mon = atoi(buf)) < 1 || (tp->tm_mon > 12))
			{
				tp->tm_mon = FALSE ;
				goto badmon ;
			}
			sp = skipw(++sp) ;
			if(!isdigit(*sp)) goto badday ;
			sp = dstring(buf,sp) ;
			tp->tm_mday = atoi(buf) ;

			sp = skipw(sp) ;
			if(*sp != '/') goto badyr ;
			sp = skipw(++sp) ;
			if(!isdigit(*sp)) goto badyr ;
			sp = dstring(buf,sp) ;
			tp->tm_year = atoi(buf) ;

			sp = gtime(sp,tp) ;
		}
		else
		{
			/*
			 * must be '12 jul 83'
			 */
			tp->tm_mday = atoi(buf) ;

			sp = extract(buf,sp) ;
			if((tp->tm_mon = find(buf,months)) < 1) goto badmon ;

			if(*sp == '.') ++sp ;
			sp = skipw(sp) ;

			if(!isdigit(*sp)) goto badyr ;
			sp = dstring(buf,sp) ;
			tp->tm_year = atoi(buf) ;

			sp = gtime(sp,tp) ;
		}
	}
	else
	{
		int flag = 0 ;	/* used to indicate looking for UNIX style */

		/*
		 * either 'jul 12 83' or (UNIX) Wed jul 12 18:33 EST 1983
		 */
		sp = extract(buf,sp) ;
		if(find(buf,dow) > 0)
		{
			sp = extract(buf,sp) ;
			++flag ;
		}

		if((tp->tm_mon = find(buf,months)) < 1) goto badmon ;

		if(*sp == '.') ++sp ;
		sp = skipw(sp) ;

		if(!isdigit(*sp)) goto badday ;
		sp = dstring(buf,sp) ;
		tp->tm_mday = atoi(buf) ;

		sp = skipw(sp) ;
		if(*sp == ',') sp++ ;
		sp = skipw(sp) ;

		if(flag) sp = skipw(gtime(sp,tp)) ;

		if(!isdigit(*sp)) goto badyr ;
		sp = dstring(buf,sp) ;
		tp->tm_year = atoi(buf) ;

		if(!flag) sp = gtime(sp,tp) ;
	}
check:
	/*
	 * check for ridiculous numbers
	 */
	if(tp->tm_mday < 1) goto badday ;
	if(tp->tm_mday > mdays[tp->tm_mon-1])
		if(!((tp->tm_mon == 2) &&	/* check for Feb 29 */
			(tp->tm_mday == 29) && (days(tp->tm_year) == 365) ))
				goto badday ;
	if(tp->tm_year >= 1900) tp->tm_year -= 1900 ;
	if(tp->tm_hour > 23)
	{
		tp->tm_hour = FALSE ;
		return(sp) ;
	}
	if(tp->tm_min > 59)
	{
		tp->tm_hour = FALSE ;
		return(sp) ;
	}
	if(tp->tm_sec > 59)
	{
		tp->tm_sec = FALSE ;
		return(sp) ;
	}
	/*
	 *	fill in day of year, day of week (these calls must be
	 *	in this order as dowk() needs doyr()
	 */

	doyr(tp) ;
	dowk(tp) ;
	/*
	 * all done !
	 */
	return(NULL) ;
badday :
	tp->tm_mday = FALSE ;
	return(sp) ;
badmon :
	return(sp) ;
badyr :
	tp->tm_year = FALSE ;
	return(sp) ;
}
/*
 *  get  hh:mm:ss
 */
char *
gtime(sp,tp) register char *sp ; register struct tm *tp ;
{
	char buf[MAXTOK],*cp ;

	sp = skipw(sp) ;
	if(isdigit(*sp))
	{
		sp = dstring(buf,sp) ;
		tp->tm_hour = atoi(buf) ;
		sp = skipw(sp) ;
		if(*sp == ':') sp = skipw(++sp) ;
		else goto out ;
		if(isdigit(*sp))
		{
			sp = dstring(buf,sp) ;
			tp->tm_min = atoi(buf) ;
			sp = skipw(sp) ;
			if(*sp == ':') sp = skipw(++sp) ;
			else goto out ;
			if(isdigit(*sp))
			{
				sp = dstring(buf,sp) ;
				tp->tm_sec = atoi(buf) ;
			}
		}
	}
out :
	sp = skipw(sp) ;
	if(isalpha(*sp))	/* PM:AM or time zone or both */
	{
		cp = extract(buf,sp) ;
		if(strcmp(buf,"am") == 0 || strcmp(buf,"pm") == 0)
		{
			if(buf[0] == 'p' && tp->tm_hour < 12)
				tp->tm_hour += 12 ;
			sp = cp = skipw(cp) ;
			cp = extract(buf,cp) ;
		}
		if(lookup(buf,tz))
		{
			if(buf[1] == 'd') tp->tm_isdst++ ;
			sp = skipw(cp) ;
		}
	}
	return(sp) ;
}
/*
 *	Ok, you were all wondering so here it is folks...
 *	THE LEAP YEAR CALCULATION
 *	note: does not take into account 1752.
 */
static days(y) 
register int y;
{
	if(y < 1970) y += 1900 ;
	if(((y % 4) == 0) && ( (y % 100) || ((y % 400)==0) )) y = 366 ;
	else y = 365 ;
	return(y) ;
}

#include <sys/types.h>
#include <sys/timeb.h>
/*
 *	translate a time.h structure to seconds since 1/1/70
 *	(ie. UNIX internal time format...GMT)
 */
long
totime(tp) 
register struct tm *tp ;
{
	long ret = 0 ;
	int i,j ;
	struct timeb tb ;

	if(tp->tm_year < 70) return(0) ;
	for(i=1970,j=tp->tm_year+1900 ; i<j ; i++)
		ret += days(i) ;
	for(i=1 ; i < tp->tm_mon ; i++)
		ret += mdays[i-1] ;
	if((days(tp->tm_year) == 366) && (tp->tm_mon > 2)) ++ret ;
	ret += (tp->tm_mday - 1) ;
	ret *= 24 ;
	ret += tp->tm_hour ;
	ret *= 60 ;
	ftime(&tb) ;
	ret += (tp->tm_min + tb.timezone) ;
	ret *= 60 ;
	ret += tp->tm_sec ;
	return(ret) ;
}
/*
 *	return day of the week
 *	of jan 1 of given year
 */

static jan1(yr)
{
	register y, d;

/*
 *	normal gregorian calendar
 *	one extra day per four years
 */

	y = yr;
	d = 4+y+(y+3)/4;

/*
 *	julian calendar
 *	regular gregorian
 *	less three days per 400
 */

	if(y > 1800) {
		d -= (y-1701)/100;
		d += (y-1601)/400;
	}

/*
 *	take care of weirdness at 1752.
 */

	if(y > 1752)
		d += 3;

	return(d%7);
}
/*
 *	given day of year and year calculate and insert day-of-week (0..6)
 */
dowk(tp) register struct tm *tp ;
{
	tp->tm_wday = (jan1(tp->tm_year+1900) + tp->tm_yday) % 7 ;
}
/*
 *	calculate day of year from year
 */
doyr(tp) register struct tm *tp ;
{
	register int i,j ;

	j = ((tp->tm_mon > 2) && (days(tp->tm_year) == 366)) ? 1 : 0 ;
	for(i=1 ; i < tp->tm_mon ; i++)
		j += mdays[i-1] ;
	tp->tm_yday = j + tp->tm_mday - 1 ;
}
