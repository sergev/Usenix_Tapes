/*
 * getadate - get arpa date.
 *
 * This routine takes a character character string containing a
 * date-time in the format specified in RFC 822 and converts it
 * to UNIX internal format.  The components of the date are left
 * in the global tm structure arpadtm.  Getarpad returns -1L if
 * the argument is syntactically incorrect.
 */

#include "config.h"
#if BSDREL == 42
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <ctype.h>

#define HR 3600
#define DAY (24L*HR)

struct tm adatetm ;

static int lookup(), getdig(), skipbl() ;
static char *d ;
static char *days[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat", 0} ;
static char *months[] = {"jan", "feb", "mar", "apr", "may", "jun",
			 "jul", "aug", "sep", "oct", "nov", "dec", 0} ;
static char *zones[] =
       {"ut", "gmt", "est", "edt", "cst", "cdt", "mst", "mdt", "pst", "pdt",
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "k", "l", "m",
	"n",   "o",   "p",   "q",   "r",   "s",
	"t",   "u",   "v",   "w",   "x",   "y",  "z",
	/* the remaining time zones are not legal according to RFC822 */
	"yst", "ydt", "hst", "hdt",
	"aest", "aesst", "acst",    "acsst",    "awst", 0} ;
static long zoffset[] =
       {0,    0,      5*HR,  4*HR,  6*HR,  5*HR,  7*HR,  6*HR,  8*HR,  7*HR,
	1*HR,2*HR,3*HR,4*HR,5*HR,6*HR,7*HR,8*HR,9*HR,10*HR,11*HR,12*HR,
	-1*HR, -2*HR, -3*HR, -4*HR, -5*HR, -6*HR,
	-7*HR, -8*HR, -9*HR,-10*HR,-11*HR,-12*HR, 0,
	/* the remaining time zones are not legal according to RFC822 */
	9*HR,  8*HR,  10*HR, 9*HR,
	-10*HR, -11*HR,  -9*HR-HR/2,-10*HR-HR/2,-8*HR,  0} ;
static int mon[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334} ;


long
getadate(date)
      register char *date ;
      {
      int i ;
      int oh ;
      long tim ;
      long offset ;

      d = date ;
      if ((adatetm.tm_wday = lookup(days)) >= 0) {
            if (getch(',') < 0)
bad:              return -1L ;
      }
      skipbl() ;
      if (! isdigit(*d))
            goto bad ;
      adatetm.tm_mday = *d++ - '0' ;
      if (isdigit(*d))
            adatetm.tm_mday = adatetm.tm_mday * 10 + *d++ - '0' ;
      if (*d == '-')		/* ARPA spec doesn't permit this */
            d++ ;
      if ((adatetm.tm_mon = lookup(months)) < 0)
            goto bad ;
      getch('-') ;		/* ARPA spec doesn't permit this */
      if ((adatetm.tm_year = getdig()) < 0)
            goto bad ;
      skipbl() ;
      if ((adatetm.tm_hour = getdig()) < 0)
            goto bad ;
      if (getch(':') < 0)
            goto bad ;
      if ((adatetm.tm_min = getdig()) < 0)
            goto bad ;
      if (getch(':') < 0)
            adatetm.tm_sec = 0 ;
      else if ((adatetm.tm_sec = getdig()) < 0)
            goto bad ;
      if (getch('+') >= 0) {
            offset = 60 ;
numtz:      if ((oh = getdig()) < 0
             || (i = getdig()) < 0)
                  goto bad ;
            offset *= 60 * oh + i ;
      } else if (getch('-') >= 0) {
            offset = -60 ;
            goto numtz ;
      } else if ((i = lookup(zones)) >= 0)
            offset = zoffset[i] ;
      else
            goto bad ;

      /* now calculate the time */
      adatetm.tm_yday = mon[adatetm.tm_mon] + adatetm.tm_mday - 1 ;
      if ((adatetm.tm_year & 03) == 0 && adatetm.tm_mon >= 2)
            adatetm.tm_yday++ ;
      tim = adatetm.tm_sec + 60 * adatetm.tm_min + 3600L * adatetm.tm_hour
          + offset + DAY * adatetm.tm_yday ;
      for (i = adatetm.tm_year ; --i >= 70 ; )
            tim += (i & 03) == 0?  366 * DAY : 365 * DAY ;
      return tim ;
}



static int
lookup(ltab)
      char **ltab ;
      {
      char tok[20] ;
      register char *dp ;
      register char *tp ;
      register char **lp ;

      dp = d ;
      tp = tok ;
      while (isspace(*dp))
            dp++ ;
      for (;;) {
            if (isupper(*dp))
                  *tp++ = tolower(*dp) ;
            else if (islower(*dp))
                  *tp++ = *dp ;
            else
                  break ;
            if (tp >= &tok[20])
                  return -1 ;
            dp++ ;
      }
      if (dp == d)
            return -1 ;
      *tp = '\0' ;
      for (lp = ltab ; *lp ; lp++) {
            if (strcmp(*lp, tok) == 0) {
                  d = dp ;
                  return lp - ltab ;
            }
      }
      return -1 ;
}


static int
getdig() {
      register char *p ;

      p = d ;
      if (! isdigit(*p) || ! isdigit(*++p))
            return -1 ;
      d += 2 ;
      return 10 * p[-1] + p[0] - (10 * '0' + '0') ;
}


static
skipbl() {
      while (isspace(*d))
            d++ ;
}


static int
getch(c) {
      skipbl() ;
      if (*d != c)
            return -1 ;
      d++ ;
      skipbl() ;
      return 0 ;
}
