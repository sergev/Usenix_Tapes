
/*********************************************************************
 *  
 *                Easter Service Routine
 *  
 *  
 * Program Copyright (c) 1987 Tigertail Associates.  All rights reserved.
 *  
 *
 *   Synopsis:
 *              easter [year]
 *
 *   If no year is specified then the date of the next Easter is printed.
 *   If a year is specified then the date of Easter that year is printed.
 *
 *********************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include "taxc.h"

#define DAY		(24L*3600L)


/*********************************************************************
 *
 *      Miscellaneous GLOBAL Values:
 *
 *********************************************************************/

char *usage[] = { "usage: %s year",
                  NULLS, };

char	*myname;			/* how program was invoked	*/

int yrday();
int easter();

/************************************************************************
 *
 *     main -- Calculate Date of Easter for a given year
 *
 ************************************************************************/

int PROC main (argc, argv)
	int	argc;
	char	**argv;

{   int y, ed, mday, month;
	static char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun" };
	long time(), now;
	char j;
	struct tm *localtime(), *gmtime(), *tp;

/*********************************************************************
 *
 *   Get year:
 *
 *********************************************************************/

	myname = *argv;

	if (argc > 2)				/* too many args given */
	then Usage();

	now = time( (long *)0 );
	tp  = localtime( &now );
	y   = tp->tm_year + 1900;

	if ( argc == 2 )
	then sscanf ( argv[1], " %d%c", &y, &j );

	if ( y < 1583 )
	then
	{    printf ( "Gregorian Easter %d is undefined.  ", y );
		 printf ( "Gregorian calendar did not exist.\n" );
		 return ( 0 );
		 }

	ed = easter ( y );

	if ( (argc < 2) && (yrday(y,ed/100,ed%100) < tp->tm_yday))
	then ed = easter ( ++y );

	month = ed / 100;
	mday  = ed % 100;
	printf( "Easter %d is %s %d",  y, months[month-1], mday );
	if ( y < 1753 )
	then printf ( " Gregorian calendar\n" );
	else printf ( "\n" );


/*********************************************************************
 *
 *     Finish up:
 *
 *********************************************************************/

	return (0);

} /* main */



/*********************************************************************
 *
 *     Procedure easter
 * 
 *     Given a year (eg 1984) it provides a date for Easter 422.
 * 
 *********************************************************************/


int PROC easter ( py )
int py;

{   /* The following amusing algorithm comes from page 5 of
     * <Practical Astronomy with your calculator>, second edition,
	 * by Peter Duffett-Smith, Cambridge University Press, 1979, 1981.
	 *
	 * Returns: 323 for March 23.
	 * Returns: 402 for April 2.
	 */

    int  a = py % 19;
	int  b = py / 100;
	int  c = py % 100;
	int  d = b / 4;
	int  e = b % 4;
	int  f = (b + 8) / 25;
	int  g = (b - f + 1) / 3;
	int  h = (19*a + b - d - g + 15) % 30;
	int  i = c / 4;
	int  k = c % 4;
	int  l = (32 + 2*e + 2*i  - h - k) % 7;
	int  m = (a + 11*h + 22*l) / 451;
	int  t = (h + l - 7*m + 114);
	int  n = t / 31;
	int  p = t % 31;

	return ( n*100 + (p+1) );
}



/************************************************************************
 *
 *  USAGE
 *
 * Print usage messages (char *usage[]) to stderr and exit nonzero.
 * Each message is followed by a newline.
 *
 ************************************************************************/

PROC Usage()
{
    register int	which = 0;		/* current line */

	while (usage [which] != NULLS)
	{   fprintf (stderr, usage [which++], myname);
	    putc ('\n', stderr);
	}

	exit (1);

} /* Usage */


/************************************************************************
 *
 *   Error
 *
 * Print an error message to stderr and exit nonzero.  Message is preceded
 * by "<myname>: " using global char *myname, and followed by a newline.
 *
 ************************************************************************/

/* VARARGS */
PROC Error (message, arg1, arg2, arg3, arg4)
	char	*message;
	long	arg1, arg2, arg3, arg4;

{   fprintf (stderr, "%s: ", myname);
	fprintf (stderr, message, arg1, arg2, arg3, arg4);
	putc ('\n', stderr);

	exit (1);

} /* Error */

